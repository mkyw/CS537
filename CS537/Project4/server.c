#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#include "udp.h"
#include "mfs.h"
#include "ufs.h"

int sd, fd, image_size;
void* start; // the head ptr of the file system in memory
super_t* super; // super block addr

void* inode_bm; // inode bitmap address
void* data_bm; // data bitmap address
void* inode_rg; // inode region address
void* data_rg; // data region address

struct sockaddr_in saddr, caddr;

unsigned long get_bit(unsigned long* bitmap, int position) {
    int index = position / 32;
    int offset = 31 - (position % 32);
    return (bitmap[index] >> offset) & 0x1;
}

void set_bit(unsigned long* bitmap, int position) {
    int index = position / 32;
    int offset = 31 - (position % 32);
    bitmap[index] |= 0x1 << offset;
}

int server_Error(message_t* reply) {
    reply->msg_type = MFS_ERROR;
    reply->inum = -1;
    UDP_Write(sd, &caddr, (char*)(void*)reply, sizeof(message_t));
    return 0;
}

int server_Stat(int inum) {
    message_t reply = {
        .msg_type = MFS_STAT
    };
    unsigned long inode_bit = get_bit(inode_bm, inum);
    if (!inode_bit) {
        server_Error(&reply);
        return -1;
    }
    inode_t* inode = inode_rg + inum * sizeof(inode_t);
    int type = inode->type;
    int size = inode->size;
    MFS_Stat_t stat;
    stat.type = type;
    stat.size = size;
    reply.inum = inum;
    memcpy(reply.buf, &stat, sizeof(MFS_Stat_t));
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;
}

void* lookup(int parent_inode_num, char* entry_name) {
    void* error_return_code = (void*)(unsigned long)-1;
    void* not_found_return_code = (void*)(unsigned long)-2;

    // Check if the parent inode is valid
    unsigned long inode_bit = get_bit(inode_bm, parent_inode_num);
    if (!inode_bit) {
        return error_return_code;
    }

    // Check if the parent inode is a directory
    inode_t* parent_inode = inode_rg + parent_inode_num * sizeof(inode_t);
    if (parent_inode->type != UFS_DIRECTORY) {
        return error_return_code;
    }

    // Get the size and number of blocks of the parent inode
    int size = parent_inode->size;
    int num_blocks = size / (UFS_BLOCK_SIZE + 1) + 1;

    // Get the data blocks of the parent inode
    int data_blocks[num_blocks];
    for (int i = 0; i < num_blocks; i++) {
        data_blocks[i] = parent_inode->direct[i];
    }

    // Get the entries in the parent inode
    int num_entries = (UFS_BLOCK_SIZE / sizeof(dir_ent_t)) * (size / UFS_BLOCK_SIZE) +
        ((size % UFS_BLOCK_SIZE) / sizeof(dir_ent_t));
    dir_ent_t* entries[num_entries];
    int remaining_entries = num_entries;
    for (int i = 0; i < num_blocks; i++) {
        int data_block = data_blocks[i];
        unsigned long data_addr = data_block * UFS_BLOCK_SIZE;
        unsigned long loop_end;
        if (remaining_entries < UFS_BLOCK_SIZE / sizeof(dir_ent_t)) {
            loop_end = remaining_entries;
        }
        else {
            loop_end = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
        }
        void* data_ptr = (void*)((unsigned long)data_addr + (unsigned long)start);
        for (int j = 0; j < loop_end; j++) {
            entries[j + (UFS_BLOCK_SIZE / sizeof(dir_ent_t)) * i] = (dir_ent_t*)data_ptr;
            remaining_entries--;
            data_ptr = (void*)data_ptr;
            data_ptr += sizeof(dir_ent_t);
        }
    }

    // Search for the requested entry in the parent inode
    for (int i = 0; i < num_entries; i++) {
        if (!strcmp(entry_name, entries[i]->name)) {
            return (void*)entries[i];
        }
    }
    return not_found_return_code;
}


int server_Lookup(int pinum, char* name) {
    message_t reply = {
        .msg_type = MFS_LOOKUP
    };
    void* ptr = lookup(pinum, name);
    if ((unsigned long)ptr == -1 || -(unsigned long)ptr == 2) {
        server_Error(&reply);
        return -1;
    }
    reply.inum = ((dir_ent_t*)ptr)->inum;
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;
}

int server_Write(int inum, char* buffer, int offset, int nbytes) {
    message_t reply = {
        .msg_type = MFS_WRITE
    };

    // Check input values
    if (nbytes > MFS_BUFFER || !get_bit(inode_bm, inum)) {
        server_Error(&reply);
        return -1;
    }
    inode_t* inode = inode_rg + inum * sizeof(inode_t);
    int size = inode->size;
    int type = inode->type;
    if (offset > size || type == MFS_DIRECTORY) {
        server_Error(&reply);
        return -1;
    }

    // Write data to blocks
    int start_blk = offset / 4096;
    int start_off = offset % 4096;
    int remain_bytes = nbytes;
    int blk_write_off = start_off;
    int buffer_off = 0;
    int this_block = inode->direct[start_blk];

    // Allocate new block if necessary
    if (this_block < 0) {
        unsigned int newblk = 0;
        for (int i = 0; i < super->num_data; i++) {
            if (!get_bit(data_bm, i)) {
                newblk = i + super->data_region_addr;
                set_bit(data_bm, i);
                this_block = newblk;
                break;
            }
        }
        if (newblk == 0) {
            server_Error(&reply);
            return -1;
        }
    }
    inode->direct[start_blk] = this_block;

    // Check if block is available for use
    if (!get_bit(data_bm, this_block - super->data_region_addr)) {
        server_Error(&reply);
        return -1;
    }
    unsigned long this_block_addr = this_block * UFS_BLOCK_SIZE;
    unsigned long cur_write_addr = this_block_addr + start_off;

    int i = 0;
    unsigned long write_addrs[nbytes];
    unsigned long data_block_use = -1;
    while (remain_bytes > 0) {
        if (blk_write_off < 4096) {
            write_addrs[buffer_off] = cur_write_addr;
            cur_write_addr += 1;
            remain_bytes -= 1;
            buffer_off += 1;
            blk_write_off += 1;
        }
        else {
            int flag = 0;
            i += 1;
            if (start_blk + i >= DIRECT_PTRS) {
                server_Error(&reply);
                return -1;
            }
            if (inode->direct[start_blk + i] != -1) {
                this_block = inode->direct[start_blk + i];
                this_block_addr = this_block * UFS_BLOCK_SIZE;
                cur_write_addr = this_block_addr;
                flag = 1;
                blk_write_off = 0;
                continue;
            }
            int has_place = 0;
            if (!flag) {
                int j = 0;
                while (j < super->num_data) {
                    if (get_bit(data_bm, j)) {
                        j++;
                        continue;
                    }
                    this_block = j + super->data_region_addr;
                    this_block_addr = this_block * UFS_BLOCK_SIZE;
                    cur_write_addr = this_block_addr;
                    blk_write_off = 0;
                    has_place = 1;
                    data_block_use = this_block;
                    break;
                }
                // no place remaining!
                if (!has_place) {
                    server_Error(&reply);
                    return -1;
                }
            }
        }
    }

    for (int i = 0; i < nbytes; i++) {
        *(char*)(void*)(unsigned long)(write_addrs[i] + (unsigned long)start) = *(buffer + i);
    }
    set_bit(data_bm, data_block_use);

    inode->size += nbytes;
    for (int i = 0; i < DIRECT_PTRS; i++) {
        if (inode->direct[i] == -1) {
            if (data_block_use != -1) {
                inode->direct[i] = data_block_use + super->data_region_addr;
            }
            break;
        }
    }
    if (msync(start, image_size, MS_SYNC) == -1) {
        server_Error(&reply);
        return -1;
    }
    reply.inum = inum;
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;
}

int server_Read(int inum, char* buffer, int offset, int nbytes) {
    message_t reply = {
        .msg_type = MFS_READ,
        .inum = inum
    };

    // Check input values
    if (nbytes > MFS_BUFFER || sizeof(buffer) > MFS_BLOCK_SIZE || !get_bit(inode_bm, inum)) {
        server_Error(&reply);
        return -1;
    }
    inode_t* inode = inode_rg + inum * sizeof(inode_t);
    int size = inode->size;
    int type = inode->type;
    if (offset >= size || (type != MFS_REGULAR_FILE && type != MFS_DIRECTORY) || (type == MFS_DIRECTORY && offset % 32 != 0)) {
        server_Error(&reply);
        return -1;
    }

    // Read data from blocks
    int start_blk = offset / 4096;
    int start_off = offset % 4096;
    int remain_bytes = nbytes;
    int blk_read_off = start_off;
    int buffer_off = 0;
    unsigned long this_block = inode->direct[start_blk];
    unsigned long this_block_addr = this_block * UFS_BLOCK_SIZE;
    unsigned long cur_read_addr = this_block_addr + start_off;

    int i = 0;
    while (remain_bytes > 0) {
        // Read data from current block
        if (blk_read_off < 4096) {
            *(buffer + buffer_off) = *(char*)(void*)(unsigned long)(cur_read_addr + (unsigned long)start);
            cur_read_addr += 1;
            remain_bytes -= 1;
            buffer_off += 1;
            blk_read_off += 1;
        }
        // Go to next block
        else {
            int flag = 0;
            while (start_blk + i < 30 && inode->direct[start_blk + i] != -1) {
                if (inode->direct[start_blk + i] != -1) {
                    this_block = inode->direct[start_blk + i];
                    this_block_addr = this_block * UFS_BLOCK_SIZE;
                    cur_read_addr = this_block_addr;
                    flag = 1;
                    blk_read_off = 0;
                    break;
                }
                i++;
            }
            if (!flag) {
                server_Error(&reply);
                return -1;
            }
        }
    }

    // Write data to reply message
    for (int i = 0; i < nbytes; i++) {
        reply.buf[i] = *(buffer + i);
    }

    // Send reply message
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;
}

int get_free_inum() {
    for (int i = 0; i < super->num_inodes; i++) {
        int bit = get_bit(inode_bm, i);
        if (bit == 0) return i;
    }
    return -1;
}

int get_free_datanum() {
    for (int i = 0; i < super->num_data; i++) {
        int bit = get_bit(data_bm, i);
        if (bit == 0) return i;
    }
    return -1;
}

int server_Create(int pinum, int type, char* name) {
    // Create a reply message with the correct message type
    message_t reply = {
        .msg_type = MFS_CREATE
    };

    // Check if the inode for the given pinum exists
    unsigned long inode_bit = get_bit(inode_bm, pinum);

    // If the inode does not exist, send an error message and return -1
    if (!inode_bit) {
        server_Error(&reply);
        return -1;
    }

    // If the name is too long, send an error message and return -1
    if (strlen(name) > 28) {
        server_Error(&reply);
        return -1;
    }

    // Check if the name already exists in the directory
    void* ptr = lookup(pinum, name);

    if ((unsigned long)ptr == -1) {
        server_Error(&reply);
        return -1;
    }

    if ((unsigned long)ptr != -2) {
        reply.inum = ((dir_ent_t*)ptr)->inum;
        UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
        return 0;
    }

    inode_t* inode_p = inode_rg + pinum * sizeof(inode_t);
    int p_type = inode_p->type;
    if (p_type != MFS_DIRECTORY) {
        server_Error(&reply);
        return -1;
    }

    // Find the last block in the directory
    int block_idx = 0;
    while ((int)inode_p->direct[block_idx] != -1) {
        block_idx++;
    }
    block_idx--;

    // Calculate the number of entries remaining in the block
    int remain_entry = (inode_p->size % (UFS_BLOCK_SIZE + 1)) / sizeof(dir_ent_t);

    // Get the block address for the last block in the directory
    unsigned long dir_block = inode_p->direct[block_idx];
    unsigned long block_addr = dir_block * UFS_BLOCK_SIZE;

    // Get the array of directory entries in the last block
    dir_ent_t* entries[remain_entry];
    void* dir_block_addr = (void*)((unsigned long)block_addr + (unsigned long)start);
    int j;
    for (j = 0; j < remain_entry; j++) {
        entries[j] = (dir_ent_t*)dir_block_addr;
        dir_block_addr = (void*)dir_block_addr;
        dir_block_addr += sizeof(dir_ent_t);
    }
    j -= 1;
    entries[remain_entry] = (dir_ent_t*)dir_block_addr;

    // If there is enough space in the last block, add the new entry
    if (sizeof(entries) < (4096 - 32)) {
        strcpy(entries[remain_entry]->name, name);
        entries[remain_entry]->inum = get_free_inum();
    }

    // If there is not enough space in the last block, create a new block
    if (sizeof(entries) >= (4096 - 32)) {
        unsigned long new_block;
        unsigned long new_block_addr;
        int has_place = 0;
        int k;
        for (k = 0; k < super->num_data; k++) {
            if (get_bit(data_bm, j))
                continue;
            new_block = k + super->data_region_addr;
            new_block_addr = new_block * UFS_BLOCK_SIZE;
            break;
        }
        if (!has_place) {
            server_Error(&reply);
            return -1;
        }
        entries[remain_entry] = (dir_ent_t*)(void*)(unsigned long)new_block_addr;
        strcpy(entries[k + 1]->name, name);
        entries[remain_entry]->inum = get_free_inum();
        set_bit(inode_bm, get_free_inum());
    }

    // Update the size of the parent directory
    inode_p->size += sizeof(dir_ent_t);
    int inode_num;

    // If the type is MFS_REGULAR_FILE, create a new regular file inode
    if (type == MFS_REGULAR_FILE) {
        inode_num = get_free_inum();
        inode_t* new_inode = inode_rg + inode_num * sizeof(inode_t);
        new_inode->type = MFS_REGULAR_FILE;
        new_inode->size = 0;
        for (int i = 0; i < 30; i++) {
            new_inode->direct[i] = -1;
        }
        set_bit(inode_bm, inode_num);
    }

    // If the type is MFS_DIRECTORY, create a new directory inode
    if (type == MFS_DIRECTORY) {
        inode_num = get_free_inum();
        inode_t* new_inode = inode_rg + inode_num * sizeof(inode_t);
        new_inode->type = MFS_DIRECTORY;
        new_inode->size = sizeof(dir_ent_t) * 2;
        for (int i = 1; i < 30; i++) {
            new_inode->direct[i] = -1;
        }

        int data_num = get_free_datanum();
        void* data_addr_ptr = (void*)(unsigned long)(data_rg + data_num * MFS_BLOCK_SIZE);

        dir_ent_t* e1 = (dir_ent_t*)data_addr_ptr;
        e1->inum = get_free_inum();
        strcpy(e1->name, ".");
        void* data_addr_ptr2 = (void*)(unsigned long)(data_rg + data_num * MFS_BLOCK_SIZE + sizeof(dir_ent_t));

        dir_ent_t* e2 = (dir_ent_t*)data_addr_ptr2;
        e2->inum = pinum;
        strcpy(e2->name, "..");

        new_inode->direct[0] = data_num + super->data_region_addr;

        set_bit(inode_bm, inode_num);
        set_bit(data_bm, data_num);
    }

    // Sync the memory-mapped file to the disk
    if (msync(start, image_size, MS_SYNC) == -1) {
        server_Error(&reply);
        return -1;
    }

    // Send the reply message with the inode number of the new entry
    reply.inum = inode_num;
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;
}

int server_Unlink(int pinum, char* name) {
    message_t reply = {
        .msg_type = MFS_UNLINK
    };

    // Get the directory entry for the file to be unlinked
    void* dir_entry = lookup(pinum, name);

    // If the returned directory entry is invalid, return error
    if ((unsigned long)dir_entry == -1 || (unsigned long)dir_entry == -2) {
        server_Error(&reply);
        return -1;
    }

    // Get the inode number of the file to be unlinked
    int rp_inum = ((dir_ent_t*)dir_entry)->inum;
    int inum = ((dir_ent_t*)dir_entry)->inum;

    // If the inode bitmap does not show the inode as being in use, return error
    if (!get_bit(inode_bm, inum)) {
        server_Error(&reply);
        return -1;
    }

    // Get the inodes for the parent directory and the file to be unlinked
    inode_t* inode_p = inode_rg + pinum * sizeof(inode_t);
    inode_t* inode_c = inode_rg + inum * sizeof(inode_t);

    // If the file to be unlinked is a non-empty directory, return error
    if (inode_c->type == UFS_DIRECTORY && inode_c->size != sizeof(dir_ent_t) * 2) {
        server_Error(&reply);
        return -1;
    }
    set_bit(inode_bm, inum);

    int i = 0;
    // Set the data block bitmap to show that all blocks used by the file are no longer in use
    for (i = 0; inode_c->direct[i] != -1; i++) {
        int data_blk = inode_c->direct[i];
        set_bit(data_bm, data_blk - super->data_region_addr);
    }

    // Get the address of the directory entry in memory and the block it is in
    dir_ent_t* cur_dir = (dir_ent_t*)dir_entry;
    dir_ent_t* next_dir = cur_dir;
    unsigned long start_dir_addr = (unsigned long)(void*)cur_dir - (unsigned long)start;
    unsigned long start_dir_block = start_dir_addr / 4096;
    unsigned long nth_block;
    for (int i = 0; i < DIRECT_PTRS; i++) {
        if (inode_p->direct[i] == start_dir_block) {
            nth_block = i;
            break;
        }
    }

    // Calculate the size difference between the beginning of the block and the directory entry, and the size of the remaining blocks
    int diff = start_dir_addr % 4096 + nth_block * 4096;
    // Calculate remaining size of parent directory
    int remainder = inode_p->size - diff;
    // Calculate current offset in block for directory entries
    int current_offset = start_dir_addr % 4096 / sizeof(dir_ent_t);
    // Get current block number for directory entries
    int n = nth_block;
    for (int i = 1; i < remainder / sizeof(dir_ent_t); i += 1) {
        n += 1;
        next_dir = (dir_ent_t*)(unsigned long)inode_p->direct[n];
        // If the value of next_dir is -1, break out of the loop
        if ((long int)next_dir == -1) {
            break;
        }
        // Copy the data from next_dir to cur_dir
        memcpy(cur_dir, next_dir, sizeof(dir_ent_t));
        cur_dir = next_dir;
        current_offset = 0;
        continue;
    }
    next_dir += 1;

    // Copy the data from next_dir to cur_dir
    memcpy(cur_dir, next_dir, sizeof(dir_ent_t));
    cur_dir += 1;
    current_offset += 1;
    inode_p->size -= sizeof(dir_ent_t);

    // Calculate the number of blocks needed for the inode based on its size
    unsigned long blk_need = inode_p->size / 4096 + 1;
    unsigned long blk_used;

    // Iterate through the direct blocks in the inode
    for (blk_used = 0; i < DIRECT_PTRS; i++) {
        if (inode_p->direct[blk_used] == -1) {
            break;
        }
    }
    if (blk_used > blk_need) {
        unsigned long blk = inode_p->direct[blk_used - 1];
        set_bit(data_bm, blk - super->data_region_addr);
        inode_p->direct[blk_used - 1] = -1;
    }

    // Sync the memory-mapped file to disk
    if (msync(start, image_size, MS_SYNC) == -1) {
        server_Error(&reply);
        return -1;
    }

    // Set the inode number in the reply message and send the reply message over the specified socket
    reply.inum = rp_inum;
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;

}

int server_Shutdown() {
    // Declare a reply message with a shutdown message type
    message_t reply = {
        .msg_type = MFS_SHUTDOWN
    };

    // Synchronize the memory and check for errors
    if (msync(start, image_size, MS_SYNC) == -1) {
        // If there is an error, set the reply message's buffer with an error message
        char str[] = "Shutdown failed: sync() failed!";
        strcpy(reply.buf, str);
        server_Error(&reply);
        return -1;
    }
    // Unmap the memory and check for errors
    if (munmap(start, image_size) == -1) {
        // If there is an error, set the reply message's buffer with an error message
        char str[] = "Shutdown failed: unmap() failed!";
        strcpy(reply.buf, str);
        server_Error(&reply);
        return -1;
    }

    // Send the reply message over the UDP connection
    UDP_Write(sd, &caddr, (char*)&reply, sizeof(message_t));
    return 0;
}

void intHandler(int dummy) {
    UDP_Close(sd);
    exit(130);
}

// server code
int main(int argc, char* argv[]) {
    signal(SIGINT, intHandler);

    if (argc != 3) return -1;
    int port = atoi(argv[1]);

    sd = UDP_Open(port);
    assert(sd > -1);
    UDP_FillSockAddr(&caddr, "localhost", port);

    fd = open(argv[2], O_RDWR);
    if (fd < 0) {
        printf("image does not exist\n");
        exit(1);
    }
    struct stat sb;
    fstat(fd, &sb);
    image_size = sb.st_size;
    start = mmap(NULL, image_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    super = start;
    inode_bm = start + super->inode_bitmap_addr * UFS_BLOCK_SIZE;
    data_bm = start + super->data_bitmap_addr * UFS_BLOCK_SIZE;
    inode_rg = start + super->inode_region_addr * UFS_BLOCK_SIZE;
    data_rg = start + super->data_region_addr * UFS_BLOCK_SIZE;

    while (1) {
        message_t msg;
        printf("server:: waiting...\n");
        int rc = UDP_Read(sd, &caddr, (char*)&msg, sizeof(message_t));
        if (rc < 0) continue;

        printf("server:: read message [size:%d contents:(%s)]\n", rc, (char*)&msg);
        switch (msg.msg_type) {
        case MFS_LOOKUP:
            server_Lookup(msg.inum, (char*)msg.buf);
            break;
        case MFS_STAT:
            server_Stat(msg.inum);
            break;
        case MFS_WRITE:
            server_Write(msg.inum, (char*)msg.buf, msg.offset, msg.nbytes);
            break;
        case MFS_READ:
            server_Read(msg.inum, (char*)msg.buf, msg.offset, msg.nbytes);
            break;
        case MFS_CREATE:
            server_Create(msg.inum, msg.creat_type, (char*)msg.buf);
            break;
        case MFS_UNLINK:
            server_Unlink(msg.inum, (char*)msg.buf);
            break;
        case MFS_SHUTDOWN:
            int ret = server_Shutdown();
            if (ret == 0)
                exit(0);
            break;
        default:
            message_t rs;
            rs.msg_type = MFS_ERROR;
            server_Error(&rs);
            break;
        }
    }
    return 0;
}

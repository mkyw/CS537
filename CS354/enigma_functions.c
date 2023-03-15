#include "enigma.h"

const char *ROTOR_CONSTANTS[] = {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ", // Identity Rotor (index 0 - and useful for testing):
        "EKMFLGDQVZNTOWYHXUSPAIBRCJ",
        "AJDKSIRUXBLHWTMCQGZNPYFVOE",
        "BDFHJLCPRTXVZNYEIWGAKMUSQO",
        "ESOVPZJAYQUIRHXLNFTGKDCMWB",
        "VZBRGITYUPSDNHLXAWMJQOFECK",
        "JPGVOUMFYQBENHZRDKASXLICTW",
        "NZJHGRCXMYSWBOUFAIVLPEKQDT",
        "FKQHTLXOCBJSPDZRAMEWNIUYGV",
};

// This method reads a character string from the keyboard and 
// stores the string in the parameter msg.
// Keyboard input will be entirely uppercase and spaces followed by 
// the enter key.  
// The string msg should contain only uppercase letters spaces and 
// terminated by the '\0' character
// Do not include the \n entered from the keyboard
void Get_Message(char msg[]){
    fgets(msg, 80, stdin);
        
    char* p = msg;
    while (*p != '\n') {
        p++;
    }
    *p = '\0';

    return;
}

// This function reads up to 4 characters from the keyboard
// These characters will be only digits 1 through 8. The user
// will press enter when finished entering the digits.
// The rotor string filled in by the function should only contain 
// the digits terminated by the '\0' character. Do not include
// the \n entered by the user. 
// The function returns the number of active rotors.
int Get_Which_Rotors(char which_rotors[]) {
    scanf("%s", which_rotors);

    // counts the number of rotors, which needs to be returned    
    int i;
    for (i = 0; which_rotors[i] != '\0'; i++);
    
    return i;
} 

// This function reads an integer from the keyboard and returns it 
// This number represents the number of rotations to apply to the 
// encryption rotors.  The input will be between 0 and 25 inclusive
int Get_Rotations(){
    int a;
        scanf("%d", &a);

        return a;
}


// This function copies the rotors indicated in the which_rotors string 
// into the encryption_rotors.  For example if which rotors contains the string 
// {'3', '1', '\0'} Then this function copies the third and first rotors into the 
// encryption rotors array in positions 0 and 1.  
// encryptions_rotors[0] = "BDFHJLCPRTXVZNYEIWGAKMUSQO"
// encryptions_rotors[1] = "EKMFLGDQVZNTOWYHXUSPAIBRCJ"
void Set_Up_Rotors(char encryption_rotors[4][27], char which_rotors[5]){
    for (int i = 0; i < strlen(which_rotors); i++) {
        strcpy(encryption_rotors[i], ROTOR_CONSTANTS[which_rotors[i] - '0']);
    }
    return;
}

// Helper method for Apply_Rotation()
void Rotation_Helper(char encryption_rotor[27]) {
        char c = encryption_rotor[25];
        for(int i = 25; i > 0; i--) {
                encryption_rotor[i] = encryption_rotor[i-1];
        }
        encryption_rotor[0] = c;
        encryption_rotor[26] = '\0';
}

// This function rotates the characters in each of the active encryption rotors
// to the right by rotations.  For example if rotations is 3 encryption_rotors[0]
// contains "BDFHJLCPRTXVZNYEIWGAKMUSQO" then after rotation this row will contain
// SQOBDFHJLCPRTXVZNYEIWGAKMU.  Apply the same rotation to all for strings in 
// encryption_rotors
void Apply_Rotation(int rotations, char encryption_rotors[4][27]) {
    // Iterates through the number of rotors and rotations needed to create the encryption rotors
    for (int j = 0; j < 4; j++) {
        for(int i = 0; i < rotations; i++) {
                        Rotation_Helper(encryption_rotors[j]);
                }
        }
    return;
}

// This function takes a string msg and applys the enigma machine to encrypt the message
// The encrypted message is stored in the string encryped_msg 
// Do not change spaces, make sure your encryped_msg is a \0 terminated string
void Encrypt(char encryption_rotors[4][27], int num_active_rotors, char msg[], char encrypted_msg[]){
    int end = 0;
    // Iterates over message until '\0' is found
    for (int i = 0; msg[i] != '\0'; i++) {
        // Keeps spaces the same
        if (msg[i] == ' ') {
            encrypted_msg[i] = ' ';
            continue;
        }

        // Encrypting method to get encrypted_msg
        char temp = msg[i];
                for (int j = 0; j < num_active_rotors; j++) {
            temp = encryption_rotors[j][(temp-'A')];
        }
        encrypted_msg[i] = temp;
        end = i;
    }

    // Sets end of string
    msg[end] = '\0';
    return;
}


// This function takes a string msg and applys the enigma machine to decrypt the message
// The encrypted message is stored in the string encryped_msg and the decrypted_message 
// is returned as a call by reference variable
// remember the encryption rotors must be used in the reverse order to decrypt a message
// Do not change spaces, make sure your decrytped_msg is a \0 terminated string
void Decrypt(char encryption_rotors[4][27], int num_active_rotors, char encrypted_msg[], char decrypted_msg[]) {
    int end = 0;
    // Iterates over message until '\0' is found
    for (int i = 0; encrypted_msg[i] != '\0'; i++) {
        // Keeps spaces the same
        if (encrypted_msg[i] == ' ') {
            decrypted_msg[i] = ' ';
            continue;
        }
        
        // Decrypting method to get message from encrypted_msg
        char temp = encrypted_msg[i];
        int counter = 0;
        for (int j = num_active_rotors-1; j >= 0; j--) {
            char* p = strchr(encryption_rotors[j], temp);
            counter = p - encryption_rotors[j];
            temp = ROTOR_CONSTANTS[0][(counter)];
        }
        decrypted_msg[i] = temp;
        end = i;
    }
    decrypted_msg[end+1] = '\0';
    return;
}

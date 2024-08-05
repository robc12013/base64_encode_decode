#ifndef base64_H
#define base64_H

int intToBinStr(int inputInt, char *binStr, int binStrLength);
int binStrToInt(char* inputStr, int inputStrLength);
void convertBitGroup(char *inputBitGroup, char *encodedGroup);
void base64Encode(unsigned char *inputBytes, char *encodedData);
void base64Decode(char *inputBytes, char *decodedData);

#endif

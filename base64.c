#include <stdio.h>
#include <math.h>
#include <string.h>
#include "./base64.h"

char *base64Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char padding = '=';

int intToBinStr(int inputInt, char *binStr, int binStrLength) {
  if (inputInt >= pow((double)2,(double)binStrLength)) {
    return 1;
  }

  for (int index = binStrLength; index >= 0; index--) {
    if (index == binStrLength) {
      binStr[index] = '\0';
    } else {
      binStr[index] = (inputInt % 2)+'0';
      inputInt /= 2;
    }
  }
}

int binStrToInt(char* inputStr, int inputStrLength) {
  double result = 0;
  int exponent = inputStrLength-1;
  for (int i = 0; i < inputStrLength; i++) {
    if (inputStr[i] == '1') {
      result += pow((double)2,(double)(exponent));
      exponent--;
    } else {
      exponent--;
    }
  }
  return (int)result;
}

// Converts groups of 24 bits into strings of 4 encoded base64 characters. [RFC4648, page 5]
void convertBitGroup(char *inputBitGroup, char *encodedGroup) {
  char sixBit[6];
  char fourBit[4];
  char twoBit[2];
  int inputBitGroupCount = 0;
  int inputBitLength = strlen(inputBitGroup);

  // Stop processing if input is larger than 24 bits.
  if (inputBitLength > 24) {
    return;
  }

  /* Break up 24-bit input to 4 groups of 6 bits. Map each 6-bit group to the corresponding base64 character.
  [RFC4648, page 5] */
  if (inputBitLength == 24) {
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 6; j++) {
        sixBit[j] = inputBitGroup[inputBitGroupCount++];
      }
      encodedGroup[i] = base64Alphabet[binStrToInt(sixBit, sizeof(sixBit))];
    }
    encodedGroup[4] = '\0';
  }

  /* Break up 16-bit input to 3 groups of 6 bits. Map each 6-bit group to the corresponding base64 character.
  Add one padding character to the end. [RFC4648, page 6]*/
  if (inputBitLength == 16) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 6; j++) {
        sixBit[j] = inputBitGroup[inputBitGroupCount++];
      }
      encodedGroup[i] = base64Alphabet[binStrToInt(sixBit, sizeof(sixBit))];
    }

    for (int k = 0; k < 4; k++) {
      fourBit[k] = inputBitGroup[inputBitGroupCount++];
    }

    encodedGroup[2] = base64Alphabet[binStrToInt(fourBit, sizeof(fourBit)) << 2];
    encodedGroup[3] = '=';
    encodedGroup[4] = '\0';
  }

  /* Break up 8-bit input to 2 groups of 6 bits. Map each 6-bit group to the corresponding base64 character.
  Add two padding characters to the end. [RFC4648, page 6]*/
  if (inputBitLength == 8) {
    for (int i = 0; i < 6; i++) {
      sixBit[i] = inputBitGroup[inputBitGroupCount++];
    }
    encodedGroup[0] = base64Alphabet[binStrToInt(sixBit, sizeof(sixBit))];

    for (int i = 0; i < 2; i++) {
      twoBit[i] = inputBitGroup[inputBitGroupCount++];
    }

    encodedGroup[1] = base64Alphabet[binStrToInt(twoBit, sizeof(twoBit)) << 4];
    encodedGroup[2] = '=';
    encodedGroup[3] = '=';
    encodedGroup[4] = '\0';
  }
}

/* Breaks up the input data into 24-bit groups, and uses the above functions to convert it to base64.
The output is then concatonated together into the ecodedData buffer. */
void base64Encode(char *inputBytes, char *encodedData) {
  int inputBytesLength = strlen(inputBytes);
  encodedData[0] = '\0';
  char bitGroup[24];
  bitGroup[0] = '\0';
  char byteInBin[9];
  int byteCounter = 0;
  char outputGroup[4];
  int inputIndex = 0;

  while (inputIndex < inputBytesLength) {
    intToBinStr(inputBytes[inputIndex],byteInBin,8);
    strcat(bitGroup, byteInBin);
    byteCounter++;

    if (byteCounter == 3) {
      convertBitGroup(bitGroup, outputGroup);
      strcat(encodedData, outputGroup);
      bitGroup[0] = '\0';
      byteCounter = 0;
    }

    inputIndex++;
  }

  if (byteCounter > 0) {
    convertBitGroup(bitGroup, outputGroup);
    strcat(encodedData, outputGroup);
  }
}

void eightBitGroupsToInt(char *eightBitGroups) {
  // Convert the byte str representations in the eightBitGroups to int.
  int eightBitGroupsLength = strlen(eightBitGroups);
  printf("eightBitGroupsLength: %d\n", eightBitGroupsLength);
  int eightBitGroupsIndex = 0;
  int newEightBitGroupsIndex = 0;
  char eightBit[8];
  eightBit[8] = '\0';
  int eightBitIndex = 0;
  while (eightBitGroupsIndex < eightBitGroupsLength) {
    eightBit[eightBitIndex++] = eightBitGroups[eightBitGroupsIndex++];
    if (eightBitIndex == 8) {
      eightBitGroups[newEightBitGroupsIndex++] = binStrToInt(eightBit, 8);
      eightBitIndex = 0;
    }
  }
  printf("newEightBitGroupsIndex: %d\n", newEightBitGroupsIndex);
  eightBitGroups[newEightBitGroupsIndex] = '\0';
}

void base64Decode(char *inputBytes, char *decodedData) {
  int inputBytesLength = strlen(inputBytes);
  decodedData[0] = '\0';
  int inputIndex = 0;
  int base64Index = 0;
  char sixBit[6];

  while (inputIndex < inputBytesLength){
    char currentChar = inputBytes[inputIndex];

    // Index of currentChar in base64 alphabet.
    for (int i = 0; i < 64; i++) {
      if (currentChar == base64Alphabet[i]) {
        base64Index = i;
        break;
      }
      if (currentChar == '=') {
        // eightBitGroupsToInt(decodedData);
        goto finalConversion;
      }
    }

    // Binary representation of the index.
    intToBinStr(base64Index, sixBit, 6);

    // Append sixBit to decodedData.
    strcat(decodedData, sixBit);

    inputIndex++;
  }

  // eightBitGroupsToInt(decodedData);
  
  finalConversion:
  int decodedDataLength = strlen(decodedData);
  printf("decodedDataLength: %d\n", decodedDataLength);
  int decodedDataIndex = 0;
  int newdecodedDataIndex = 0;
  char eightBit[8];
  eightBit[8] = '\0';
  int eightBitIndex = 0;
  while (decodedDataIndex < decodedDataLength) {
    eightBit[eightBitIndex++] = decodedData[decodedDataIndex++];
    if (eightBitIndex == 8) {
      decodedData[newdecodedDataIndex++] = binStrToInt(eightBit, 8);
      eightBitIndex = 0;
    }
  }
  printf("newdecodedDataIndex: %d\n", newdecodedDataIndex);
  decodedData[newdecodedDataIndex] = '\0';
}

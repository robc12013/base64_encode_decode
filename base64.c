#include <stdio.h>
#include <math.h>
#include <string.h>
#include "./base64.h"
#include "./logger.h"

char *base64Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char padding = '=';

int intToBinStr(int inputInt, char *binStr, int sizeOfBinStr) {
  // If the int is bigger than what the binStr can hold, stop execution.
  if (inputInt >= pow(2.0,(double)sizeOfBinStr-1)) {
    return 1;
  }

  // Populate the binStr array with a binary representation of the inputInt.
  for (int index = sizeOfBinStr-2; index >= 0; index--) {
    binStr[index] = (inputInt % 2)+'0';
    inputInt /= 2;
  }

  // Add a null on the end of binStr so str functions know where the char array ends.
  binStr[sizeOfBinStr-1] = '\0';

  return 0;
}

int binStrToInt(char* inputStr, int strLenInput) {
  double result = 0;
  int numOfBits = strLenInput;
  for (int i = 0; i < strLenInput; i++) {
    if (inputStr[i] == '1') {
      result += pow(2.0,(double)(--numOfBits));
    } else {
      numOfBits--;
    }
  }
  return (int)result;
}

// Converts groups of 24 bits into strings of 4 encoded base64 characters. [RFC4648, page 5]
void convertBitGroup(char *inputBitGroup, char *encodedGroup) {
  char sixBit[7];
  char fourBit[5];
  char twoBit[3];
  int inputBitGroupCount = 0;
  int inputBitLength = strlen(inputBitGroup);

  // Stop processing if input is larger than 24 bits.
  if (inputBitLength > 24) {
    return;
  }

  /* Break up 24-bit input to 4 groups of 6 bits. Map each 6-bit group to the corresponding base64 character.
  [RFC4648, page 5] */
  if (inputBitLength == 24) {
    // Break up 24 bit group into four 6 bit groups.
    for (int i = 0; i < 4; i++) {
      for (int j = 0; j < 6; j++) {
        sixBit[j] = inputBitGroup[inputBitGroupCount++];
      }
      sixBit[6] = '\0';
      encodedGroup[i] = base64Alphabet[binStrToInt(sixBit, strlen(sixBit))];
    }
    encodedGroup[4] = '\0';
    return;
  }

  /* Break up 16-bit input to 3 groups of 6 bits. Map each 6-bit group to the corresponding base64 character.
  Add one padding character to the end. [RFC4648, page 6]*/
  if (inputBitLength == 16) {
    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 6; j++) {
        sixBit[j] = inputBitGroup[inputBitGroupCount++];
      }
      sixBit[6] = '\0';
      encodedGroup[i] = base64Alphabet[binStrToInt(sixBit, strlen(sixBit))];
    }

    for (int k = 0; k < 4; k++) {
      fourBit[k] = inputBitGroup[inputBitGroupCount++];
    }
    fourBit[4] = '\0';

    encodedGroup[2] = base64Alphabet[binStrToInt(fourBit, strlen(fourBit)) << 2];
    encodedGroup[3] = '=';
    encodedGroup[4] = '\0';
    return;
  }

  /* Break up 8-bit input to 2 groups of 6 bits. Map each 6-bit group to the corresponding base64 character.
  Add two padding characters to the end. [RFC4648, page 6]*/
  if (inputBitLength == 8) {
    for (int i = 0; i < 6; i++) {
      sixBit[i] = inputBitGroup[inputBitGroupCount++];
    }
    sixBit[6] = '\0';
    encodedGroup[0] = base64Alphabet[binStrToInt(sixBit, strlen(sixBit))];

    for (int i = 0; i < 2; i++) {
      twoBit[i] = inputBitGroup[inputBitGroupCount++];
    }
    twoBit[2] = '\0';

    encodedGroup[1] = base64Alphabet[binStrToInt(twoBit, strlen(twoBit)) << 4];
    encodedGroup[2] = '=';
    encodedGroup[3] = '=';
    encodedGroup[4] = '\0';
  }
}

/* Breaks up the input data into 24-bit groups, and uses the above functions to convert it to base64.
The output is then concatonated together into the ecodedData buffer. */
void base64Encode(unsigned char *inputBytes, char *encodedData) {
  int inputBytesLength = strlen((char *)inputBytes);
  encodedData[0] = '\0';
  char bitGroup[25];
  bitGroup[0] = '\0';
  char byteInBin[9];
  int byteCounter = 0;
  char outputGroup[5];
  int inputIndex = 0;

  while (inputIndex < inputBytesLength) {
    intToBinStr(inputBytes[inputIndex],byteInBin,sizeof(byteInBin));
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

void base64Decode(char *inputBytes, char *decodedData) {
  int inputBytesLength = strlen(inputBytes);
  decodedData[0] = '\0';
  int inputIndex = 0;
  int base64Index = 0;
  char sixBit[7];

  while (inputIndex < inputBytesLength){
    char currentChar = inputBytes[inputIndex];

    // Index of currentChar in base64 alphabet.
    for (int i = 0; i < 64; i++) {
      if (currentChar == base64Alphabet[i]) {
        base64Index = i;
        break;
      }
      if (currentChar == '=') {
        
        goto finalConversion;
      }
    }

    // Binary representation of the index.
    intToBinStr(base64Index, sixBit, sizeof(sixBit));

    // Append sixBit to decodedData.
    strcat(decodedData, sixBit);

    inputIndex++;
  }
  
  finalConversion:
  int decodedDataLength = strlen(decodedData);
  int decodedDataIndex = 0;
  int newdecodedDataIndex = 0;
  char eightBit[9];
  eightBit[8] = '\0';
  int eightBitIndex = 0;
  while (decodedDataIndex < decodedDataLength) {
    eightBit[eightBitIndex++] = decodedData[decodedDataIndex++];
    if (eightBitIndex == 8) {
      decodedData[newdecodedDataIndex++] = binStrToInt(eightBit, strlen(eightBit));
      eightBitIndex = 0;
    }
  }

  decodedData[newdecodedDataIndex] = '\0';
  return;
}

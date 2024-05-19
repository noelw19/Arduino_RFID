#### Note: 
  so far only seems to work with mifare classic 1k tags and cards.

#### using: 

1. rfid rc522 module
2. rgb light
3. button

#### has 4 modes
- authenticate
- write
- read 
- dump 

#### light: 
- green = completed
- red = error
- yellow = working on it 

## Reading Mode:
I have set it up so that it looks at the first block in each sector of memory, checks if this block has any data, if it has data it will convert this data to readble ascii.
  
## Authentication Mode:
simply checks the UID of the card being scanned and allows or rejects according to UID.
  
## Dump Mode: 
just dumps entire contents of the card in hexidecimal format, function defined in mfrc522 arduino library
  
## Write Mode:
unimplemented.

## Pin Placement:
### RFID pins
- vcc : 3.3
- rst : 9
- gnd : gnd
- mios : 12
- mosi : 11
- sck : 13
- nss : 10
### Button
- btn : 2
### RGB Light
- rbg lights
- r : 6
- g : 5
- b : 4

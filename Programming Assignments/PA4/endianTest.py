#little python script I got from the internet that tells the machine's endianness
from sys import byteorder
print(byteorder)
# will print 'little' if little endian
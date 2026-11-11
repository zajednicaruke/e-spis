CC = clang
CFLAGS = -std=c2x -O2 -I../include
ecdsa_p256_test: ecdsa_p256.c ecdsa_p256_test.c
$(CC) $(CFLAGS) ecdsa_p256.c ecdsa_p256_test.c ../src/bignum.c ../modinv/modinv.c -o ecdsa_p256_test
clean:
rm -f ecdsa_p256_test

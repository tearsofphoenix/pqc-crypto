//
// Created by Isaac on 2018/1/15.
//

#include "fft-test.h"
#include "cipher-test.h"
#include "array-test.h"
#include "key-test.h"


int main() {

    fft_test_all(1);

    cipher_test();

    array_test();

//    key_test();

    return 0;
}
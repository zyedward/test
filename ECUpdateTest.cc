/**
 * Copyright (c) 2017-2018 East China Normal University.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <isa-l.h>
#include <algorithm>
#include <cstring>
#include <iostream>

#include <vector>

const int CHUNK_NUM = 3; // the number of data blocks
const int PARITY_NUM = 2;// the number of parity blocks

bool
encode(int length, uint8_t* data[], uint8_t* coding[])
{
    std::vector<uint8_t> encode_matrix((CHUNK_NUM + PARITY_NUM) * CHUNK_NUM);
    std::vector<uint8_t> table(32 * CHUNK_NUM * PARITY_NUM);
    // Generate encode matrix
    gf_gen_cauchy1_matrix(encode_matrix.data(), (CHUNK_NUM + PARITY_NUM),
                          CHUNK_NUM);
    // Generates the expanded tables needed for fast encoding
    ec_init_tables(CHUNK_NUM, PARITY_NUM, 
                   &encode_matrix[CHUNK_NUM * CHUNK_NUM], table.data());
    // Actually generated the error correction codes
    ec_encode_data(length, CHUNK_NUM, PARITY_NUM, table.data(),  
                   static_cast<uint8_t**>(data),
                   static_cast<uint8_t**>(&coding[0]));
    return true;
}


/*
 * TEST
 * ISA-L ec_encode_data_update()
 * params:
 *  len: the length of data or parity blocks
 *  vec_i: the vector index of updated data block
 *  data: the pointer to updated data block increment
 *  coding: the pointer to parity blocks
 * */
void
updateTest(int len, int vec_i, uint8_t *data, uint8_t **coding)
{ 
    std::vector<uint8_t> encode_matrix((CHUNK_NUM + PARITY_NUM) * CHUNK_NUM);
    std::vector<uint8_t> table(32 * CHUNK_NUM * PARITY_NUM);
    // Generate encode matrix
    gf_gen_cauchy1_matrix(encode_matrix.data(), (CHUNK_NUM + PARITY_NUM),
                          CHUNK_NUM);
    // Generates the expanded tables needed for fast encoding
    ec_init_tables(CHUNK_NUM, PARITY_NUM, 
                   &encode_matrix[CHUNK_NUM * CHUNK_NUM], table.data());
    ec_encode_data_update(len, CHUNK_NUM, PARITY_NUM, vec_i, table.data(), data, coding);

}

void
print(int k, int len, uint8_t** sources) {
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < len; j++){
            printf("%I8u ", sources[i][j]);
        }
        printf("\n");
        
    }
}

int
main(int argc, char*argv[]) {
    int len = 5;                           // the length of data or parity blocks
    uint8_t* data[CHUNK_NUM];              //data blocks
    uint8_t* coding[PARITY_NUM];           //parity blocks
    for (int i = 0; i < CHUNK_NUM; i++) {
        data[i] = NULL;
        data[i] = new uint8_t[len];
        for (int j = 0; j < len; j++){
            data[i][j] = static_cast<uint8_t>(i);
        }
    }

    for (int i = 0; i < PARITY_NUM; i++) {
        coding[i] = NULL;
        coding[i] = new uint8_t[len];
        memset(coding[i], 0, len);
    }
    printf("data block\n");
    print(CHUNK_NUM, len, data);
    for (int i = 0; i < CHUNK_NUM; i++) {
        updateTest(len, i, data[i], coding);
    }
    printf("encode result\n");
    print(PARITY_NUM, len, coding);
    printf("update data[1][0] = 9\n");
    uint8_t* temp = new uint8_t[len];
    memset(temp, 0, len);
    temp[0] = 9;
    temp[0] ^= data[1][0];
    data[1][0] = 9;
    updateTest(len, 1, temp, coding);
    printf("increment update result\n");
    print(PARITY_NUM, len, coding);
    encode(len, data, coding);
    printf("encode result\n");
    print(PARITY_NUM, len, coding);
    for(int i = 0 ; i < CHUNK_NUM; i++){
        delete [] data[i];
    }
    for(int i = 0; i < PARITY_NUM;i++){
        delete [] coding[i];
    }
    printf("SUCCESS\n");
    return 0;
}

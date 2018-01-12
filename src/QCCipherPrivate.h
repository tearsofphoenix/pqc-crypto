//
// Created by Isaac on 2018/1/12.
//

#ifndef PQC_CRYPTO_QCCIPHERPRIVATE_H
#define PQC_CRYPTO_QCCIPHERPRIVATE_H

#include "QCCipher.h"

struct QCCipher {
    QCOBJECTFIELDS
    QCKeyRef privateKey;
    QCKeyRef publicKey;
};

#endif //PQC_CRYPTO_QCCIPHERPRIVATE_H

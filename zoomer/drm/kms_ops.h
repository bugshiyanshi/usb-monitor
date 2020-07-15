#ifndef __KMS_OPS_H__
#define __KMS_OPS_H__

#include "kms_config.h"


int kms_init(int fd, struct kms_config **kms);
int kms_set(int fd, struct kms_config *kms);
struct format* kms_get_format(struct kms_config *kms);

#endif


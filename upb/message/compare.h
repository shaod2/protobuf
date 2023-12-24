// Protocol Buffers - Google's data interchange format
// Copyright 2023 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef UPB_MESSAGE_COMPARE_H_
#define UPB_MESSAGE_COMPARE_H_

#include <stddef.h>

#include "upb/message/message.h"
#include "upb/message/value.h"
#include "upb/mini_table/extension.h"
#include "upb/mini_table/field.h"
#include "upb/mini_table/message.h"

// Must be last.
#include "upb/port/def.inc"

#define kUpb_BaseField_Begin ((size_t)-1)
#define kUpb_Extension_Begin ((size_t)-1)

#ifdef __cplusplus
extern "C" {
#endif

// Iterate over (only) base fields.
UPB_API bool upb_Message_NextBaseField(const upb_Message* msg,
                                       const upb_MiniTable* m,
                                       const upb_MiniTableField** out_f,
                                       upb_MessageValue* out_v, size_t* iter);

// Iterate over (only) extensions.
UPB_API bool upb_Message_NextExtension(const upb_Message* msg,
                                       const upb_MiniTable* m,
                                       const upb_MiniTableExtension** out_e,
                                       upb_MessageValue* out_v, size_t* iter);

// Returns true if no known fields or extensions are set in the message.
UPB_API bool upb_Message_IsEmpty(const upb_Message* msg,
                                 const upb_MiniTable* m);

UPB_API bool upb_Message_IsEqual(const upb_Message* msg1,
                                 const upb_Message* msg2,
                                 const upb_MiniTable* m);

// Compares two messages by serializing them and calling memcmp().
UPB_API bool upb_Message_IsExactlyEqual(const upb_Message* msg1,
                                        const upb_Message* msg2,
                                        const upb_MiniTable* m);

#ifdef __cplusplus
} /* extern "C" */
#endif

#include "upb/port/undef.inc"

#endif  // UPB_MESSAGE_COMPARE_H_

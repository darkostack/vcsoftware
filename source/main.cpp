/*
 * Copyright (c) 2020, Vertexcom Technologies, Inc.
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains
 * the property of Vertexcom Technologies, Inc. and its suppliers,
 * if any. The intellectual and technical concepts contained
 * herein are proprietary to Vertexcom Technologies, Inc.
 * and may be covered by U.S. and Foreign Patents, patents in process,
 * and protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Vertexcom Technologies, Inc.
 *
 * Authors: Darko Pancev <darko.pancev@vertexcom.com>
 */

#include "core/code_utils.h"
#include "core/instance.hpp"
#include "core/new.hpp"

#include "main.hpp"

static DEFINE_ALIGNED_VAR(main_raw, sizeof(Main), uint64_t);

int main(void)
{
    void *instance = static_cast<void *>(instance_get());

    Main *base = new (&main_raw) Main(instance);

    base->setup();

    while (1)
    {
        base->loop();
    }

    return 0;
}

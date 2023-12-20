#! /bin/sh
# SPDX-FileCopyrightText: none
# SPDX-License-Identifier: BSD-3-Clause
$XGETTEXT `find . -name "*.cpp" -o -name "*.h"` -o $podir/libkimap6.pot

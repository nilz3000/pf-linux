// SPDX-License-Identifier: MIT
/*
 * Copyright © 2021 Intel Corporation
 */

#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/string.h>

#include "i915_mitigations.h"

static unsigned long mitigations = ~0UL;

enum {
	CLEAR_RESIDUALS = 0,
};

bool i915_mitigate_clear_residuals(void)
{
	return READ_ONCE(mitigations) & BIT(CLEAR_RESIDUALS);
}

static int mitigations_parse(const char *arg)
{
	unsigned long new = ~0UL;
	char *str, *sep, *tok;
	bool first = true;
	int err = 0;

	str = kstrdup(arg, GFP_KERNEL);
	if (!str)
		return -ENOMEM;

	for (sep = strim(str); (tok = strsep(&sep, ","));) {
		bool enable = true;

		if (first) {
			first = false;

			if (!strcmp(tok, "auto")) {
				new = ~0UL;
				continue;
			}

			new = 0;
			if (!strcmp(tok, "off"))
				continue;
		}

		if (*tok == '!') {
			enable = !enable;
			tok++;
		}

		if (!strncmp(tok, "no", 2)) {
			enable = !enable;
			tok += 2;
		}

		if (*tok == '\0')
			continue;

		if (!strcmp(tok, "residuals")) {
			if (enable)
				new |= BIT(CLEAR_RESIDUALS);
			else
				new &= ~BIT(CLEAR_RESIDUALS);
		} else {
			err = -EINVAL;
			break;
		}
	}
	kfree(str);
	if (err)
		return err;

	WRITE_ONCE(mitigations, new);
	return 0;
}

static int mitigations_set(const char *val, const struct kernel_param *kp)
{
	int err;

	err = mitigations_parse(val);
	if (err)
		return err;

	err = param_set_charp(val, kp);
	if (err)
		return err;

	return 0;
}

static const struct kernel_param_ops ops = {
	.set = mitigations_set,
	.get = param_get_charp,
	.free = param_free_charp
};

static char *param;
module_param_cb_unsafe(mitigations, &ops, &param, 0400);
MODULE_PARM_DESC(mitigations,
"Selectively enable security mitigations for all Intel® GPUs.\n"
"\n"
"  auto -- enables all mitigations required for the platform [default]\n"
"  off  -- disables all mitigations\n"
"\n"
"Individual mitigations can be enabled by passing a comma-separated string,\n"
"e.g. mitigations=residuals to enable only clearing residuals or\n"
"mitigations=auto,noresiduals to disable only the clear residual mitigation.\n"
"Either '!' or 'no' may be used to switch from enabling the mitigation to\n"
"disabling it.\n"
"\n"
"Active mitigations for Ivybridge, Baytrail, Haswell:\n"
"  residuals -- clear all thread-local registers between contexts"
);

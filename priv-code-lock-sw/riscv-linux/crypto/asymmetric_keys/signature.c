/* Signature verification with an asymmetric key
 *
 * See Documentation/security/asymmetric-keys.txt
 *
 * Copyright (C) 2012 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version
 * 2 of the Licence, or (at your option) any later version.
 */

#define pr_fmt(fmt) "SIG: "fmt
#include <keys/asymmetric-subtype.h>
#include <linux/export.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <crypto/public_key.h>
#include "asymmetric_keys.h"

#define __m_s __section(.module)

/*
 * Destroy a public key signature.
 */
void __m_s public_key_signature_free(struct public_key_signature *sig)
{
	int i;

	if (sig) {
		for (i = 0; i < ARRAY_SIZE(sig->auth_ids); i++)
			kfree(sig->auth_ids[i]);
		kfree(sig->s);
		kfree(sig->digest);
		kfree(sig);
	}
}
EXPORT_SYMBOL_GPL(public_key_signature_free);

/**
 * verify_signature - Initiate the use of an asymmetric key to verify a signature
 * @key: The asymmetric key to verify against
 * @sig: The signature to check
 *
 * Returns 0 if successful or else an error.
 */
int __m_s verify_signature(const struct key *key,
		     const struct public_key_signature *sig)
{
	const struct asymmetric_key_subtype *subtype;
	int ret;

	pr_devel("==>%s()\n", __func__);

	if (key->type != &key_type_asymmetric)
		return -EINVAL;
	subtype = asymmetric_key_subtype(key);
	if (!subtype ||
	    !key->payload.data[0])
		return -EINVAL;
	if (!subtype->verify_signature)
		return -ENOTSUPP;

	ret = subtype->verify_signature(key, sig);

	pr_devel("<==%s() = %d\n", __func__, ret);
	return ret;
}
EXPORT_SYMBOL_GPL(verify_signature);

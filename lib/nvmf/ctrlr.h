/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef SPDK_NVMF_CTRLR_H
#define SPDK_NVMF_CTRLR_H

#include "spdk/stdinc.h"

#include "spdk/nvmf_spec.h"
#include "spdk/queue.h"

struct spdk_bdev;

/* define a virtual controller limit to the number of QPs supported */
#define MAX_QPAIRS_PER_CTRLR 64

struct spdk_nvmf_transport;
struct spdk_nvmf_request;

enum spdk_nvmf_qpair_type {
	QPAIR_TYPE_AQ = 0,
	QPAIR_TYPE_IOQ = 1,
};

struct spdk_nvmf_qpair {
	struct spdk_nvmf_transport		*transport;
	struct spdk_nvmf_ctrlr			*ctrlr;
	enum spdk_nvmf_qpair_type		type;

	uint16_t				qid;
	uint16_t				sq_head;
	uint16_t				sq_head_max;

	TAILQ_ENTRY(spdk_nvmf_qpair) 		link;
};

/*
 * This structure represents an NVMe-oF controller,
 * which is like a "session" in networking terms.
 */
struct spdk_nvmf_ctrlr {
	uint16_t			cntlid;
	struct spdk_nvmf_subsystem 	*subsys;

	struct {
		union spdk_nvme_cap_register	cap;
		union spdk_nvme_vs_register	vs;
		union spdk_nvme_cc_register	cc;
		union spdk_nvme_csts_register	csts;
	} vcprop; /* virtual controller properties */

	TAILQ_HEAD(, spdk_nvmf_qpair) qpairs;
	int num_qpairs;
	int max_qpairs_allowed;
	uint32_t kato;
	union {
		uint32_t raw;
		struct {
			union spdk_nvme_critical_warning_state crit_warn;
			uint8_t ns_attr_notice : 1;
			uint8_t fw_activation_notice : 1;
		} bits;
	} async_event_config;
	struct spdk_nvmf_request *aer_req;
	uint8_t hostid[16];
	struct spdk_nvmf_poll_group		*group;

	TAILQ_ENTRY(spdk_nvmf_ctrlr) 		link;
};

void spdk_nvmf_ctrlr_connect(struct spdk_nvmf_qpair *qpair,
			     struct spdk_nvmf_fabric_connect_cmd *cmd,
			     struct spdk_nvmf_fabric_connect_data *data,
			     struct spdk_nvmf_fabric_connect_rsp *rsp);

struct spdk_nvmf_qpair *spdk_nvmf_ctrlr_get_qpair(struct spdk_nvmf_ctrlr *ctrlr, uint16_t qid);

void
spdk_nvmf_property_get(struct spdk_nvmf_ctrlr *ctrlr,
		       struct spdk_nvmf_fabric_prop_get_cmd *cmd,
		       struct spdk_nvmf_fabric_prop_get_rsp *response);

void
spdk_nvmf_property_set(struct spdk_nvmf_ctrlr *ctrlr,
		       struct spdk_nvmf_fabric_prop_set_cmd *cmd,
		       struct spdk_nvme_cpl *rsp);

int spdk_nvmf_ctrlr_poll(struct spdk_nvmf_ctrlr *ctrlr);

void spdk_nvmf_ctrlr_destruct(struct spdk_nvmf_ctrlr *ctrlr);

int spdk_nvmf_ctrlr_process_admin_cmd(struct spdk_nvmf_request *req);

int spdk_nvmf_ctrlr_process_io_cmd(struct spdk_nvmf_request *req);

bool spdk_nvmf_ctrlr_dsm_supported(struct spdk_nvmf_ctrlr *ctrlr);

int spdk_nvmf_bdev_ctrlr_identify_ns(struct spdk_bdev *bdev, struct spdk_nvme_ns_data *nsdata);

#endif

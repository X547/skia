/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef GrAuditTrail_DEFINED
#define GrAuditTrail_DEFINED

#include "include/gpu/GrTypes.h"

#include "include/core/SkRect.h"
#include "include/core/SkString.h"
#include "include/gpu/GrConfig.h"
#include "include/private/SkTArray.h"
#include "src/core/SkTHash.h"
#include "src/gpu/ganesh/GrRenderTargetProxy.h"

class GrOp;
class SkJSONWriter;

/*
 * GrAuditTrail collects a list of draw ops, detailed information about those ops, and can dump them
 * to json.
 *
 * Capturing this information is expensive and consumes a lot of memory, therefore it is important
 * to enable auditing only when required and disable it promptly. The AutoEnable class helps to
 * ensure that the audit trail is disabled in a timely fashion. Once the information has been dealt
 * with, be sure to call reset(), or the log will simply keep growing.
 */
class GrAuditTrail {
public:
    GrAuditTrail() : fClientID(kGrAuditTrailInvalidID), fEnabled(false) {}

    class AutoEnable {
    public:
        AutoEnable(GrAuditTrail* auditTrail)
            : fAuditTrail(auditTrail) {
            SkASSERT(!fAuditTrail->isEnabled());
            fAuditTrail->setEnabled(true);
        }

        ~AutoEnable() {
            SkASSERT(fAuditTrail->isEnabled());
            fAuditTrail->setEnabled(false);
        }

    private:
        GrAuditTrail* fAuditTrail;
    };

    class AutoManageOpsTask {
    public:
        AutoManageOpsTask(GrAuditTrail* auditTrail)
                : fAutoEnable(auditTrail), fAuditTrail(auditTrail) {}

        ~AutoManageOpsTask() { fAuditTrail->fullReset(); }

    private:
        AutoEnable fAutoEnable;
        GrAuditTrail* fAuditTrail;
    };

    class AutoCollectOps {
    public:
        AutoCollectOps(GrAuditTrail* auditTrail, int clientID)
                : fAutoEnable(auditTrail), fAuditTrail(auditTrail) {
            fAuditTrail->setClientID(clientID);
        }

        ~AutoCollectOps() { fAuditTrail->setClientID(kGrAuditTrailInvalidID); }

    private:
        AutoEnable fAutoEnable;
        GrAuditTrail* fAuditTrail;
    };

    void pushFrame(const char* framename) {
        SkASSERT(fEnabled);
        fCurrentStackTrace.push_back(SkString(framename));
    }

    void addOp(const GrOp*, GrRenderTargetProxy::UniqueID proxyID);

    void opsCombined(const GrOp* consumer, const GrOp* consumed);

    // Because op combining is heavily dependent on sequence of draw calls, these calls will only
    // produce valid information for the given draw sequence which preceeded them. Specifically, ops
    // of future draw calls may combine with previous ops and thus would invalidate the json. What
    // this means is that for some sequence of draw calls N, the below toJson calls will only
    // produce JSON which reflects N draw calls. This JSON may or may not be accurate for N + 1 or
    // N - 1 draws depending on the actual combining algorithm used.
    void toJson(SkJSONWriter& writer) const;

    // returns a json string of all of the ops associated with a given client id
    void toJson(SkJSONWriter& writer, int clientID) const;

    bool isEnabled() { return fEnabled; }
    void setEnabled(bool enabled) { fEnabled = enabled; }

    void setClientID(int clientID) { fClientID = clientID; }

    // We could just return our internal bookkeeping struct if copying the data out becomes
    // a performance issue, but until then its nice to decouple
    struct OpInfo {
        struct Op {
            int    fClientID;
            SkRect fBounds;
        };

        SkRect                   fBounds;
        GrSurfaceProxy::UniqueID fProxyUniqueID;
        SkTArray<Op>             fOps;
    };

    void getBoundsByClientID(SkTArray<OpInfo>* outInfo, int clientID);
    void getBoundsByOpsTaskID(OpInfo* outInfo, int opsTaskID);

    void fullReset();

    static const int kGrAuditTrailInvalidID;

private:
    // TODO if performance becomes an issue, we can move to using SkVarAlloc
    struct Op {
        void toJson(SkJSONWriter& writer) const;
        SkString fName;
        SkTArray<SkString> fStackTrace;
        SkRect fBounds;
        int fClientID;
        int fOpsTaskID;
        int fChildID;
    };
    typedef SkTArray<std::unique_ptr<Op>, true> OpPool;

    typedef SkTArray<Op*> Ops;

    struct OpNode {
        OpNode(const GrSurfaceProxy::UniqueID& proxyID) : fProxyUniqueID(proxyID) { }
        void toJson(SkJSONWriter& writer) const;

        SkRect                         fBounds;
        Ops                            fChildren;
        const GrSurfaceProxy::UniqueID fProxyUniqueID;
    };
    typedef SkTArray<std::unique_ptr<OpNode>, true> OpsTask;

    void copyOutFromOpsTask(OpInfo* outOpInfo, int opsTask);

    template <typename T>
    static void JsonifyTArray(SkJSONWriter& writer, const char* name, const T& array);

    OpPool fOpPool;
    SkTHashMap<uint32_t, int> fIDLookup;
    SkTHashMap<int, Ops*> fClientIDLookup;
    OpsTask fOpsTask;
    SkTArray<SkString> fCurrentStackTrace;

    // The client can pass in an optional client ID which we will use to mark the ops
    int fClientID;
    bool fEnabled;
};

#define GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, invoke, ...) \
        if (audit_trail->isEnabled()) audit_trail->invoke(__VA_ARGS__)

#define GR_AUDIT_TRAIL_AUTO_FRAME(audit_trail, framename) \
    GR_AUDIT_TRAIL_INVOKE_GUARD((audit_trail), pushFrame, framename)

#define GR_AUDIT_TRAIL_ADD_OP(audit_trail, op, proxy_id) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, addOp, op, proxy_id)

#define GR_AUDIT_TRAIL_OPS_RESULT_COMBINED(audit_trail, combineWith, op) \
    GR_AUDIT_TRAIL_INVOKE_GUARD(audit_trail, opsCombined, combineWith, op)

#endif // GrAuditTrail_DEFINED

/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"

using sk_gpu_test::GrContextFactory;
using sk_gpu_test::GLTestContext;
using sk_gpu_test::ContextInfo;

// TODO: currently many GPU tests are declared outside SK_SUPPORT_GPU guards.
// Thus we export the empty RunWithGPUTestContexts when SK_SUPPORT_GPU=0.
namespace skiatest {

#if SK_SUPPORT_GPU
bool IsGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return kOpenGL_GrBackend == GrContextFactory::ContextTypeBackend(type);
}
bool IsVulkanContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return kVulkan_GrBackend == GrContextFactory::ContextTypeBackend(type);
}
bool IsRenderingGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return IsGLContextType(type) && GrContextFactory::IsRenderingContext(type);
}
bool IsNullGLContextType(sk_gpu_test::GrContextFactory::ContextType type) {
    return type == GrContextFactory::kNullGL_ContextType;
}
#else
bool IsGLContextType(int) { return false; }
bool IsVulkanContextType(int) { return false; }
bool IsRenderingGLContextType(int) { return false; }
bool IsNullGLContextType(int) { return false; }
#endif

void RunWithGPUTestContexts(GrContextTestFn* test, GrContextTypeFilterFn* contextTypeFilter,
                            Reporter* reporter, GrContextFactory* factory) {
#if SK_SUPPORT_GPU

#if defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_WIN) || defined(SK_BUILD_FOR_MAC)
    static constexpr auto kNativeGLType = GrContextFactory::kGL_ContextType;
#else
    static constexpr auto kNativeGLType = GrContextFactory::kGLES_ContextType;
#endif

    for (int typeInt = 0; typeInt < GrContextFactory::kContextTypeCnt; ++typeInt) {
        GrContextFactory::ContextType contextType = (GrContextFactory::ContextType) typeInt;
        // Use "native" instead of explicitly trying OpenGL and OpenGL ES. Do not use GLES on
        // desktop since tests do not account for not fixing http://skbug.com/2809
        if (contextType == GrContextFactory::kGL_ContextType ||
            contextType == GrContextFactory::kGLES_ContextType) {
            if (contextType != kNativeGLType) {
                continue;
            }
        }
        ContextInfo ctxInfo = factory->getContextInfo(contextType,
                                                  GrContextFactory::ContextOverrides::kDisableNVPR);
        if (contextTypeFilter && !(*contextTypeFilter)(contextType)) {
            continue;
        }

        ReporterContext ctx(reporter, SkString(GrContextFactory::ContextTypeName(contextType)));
        if (ctxInfo.grContext()) {
            (*test)(reporter, ctxInfo);
            ctxInfo.grContext()->flush();
        }
        ctxInfo = factory->getContextInfo(contextType,
                                          GrContextFactory::ContextOverrides::kRequireNVPRSupport);
        if (ctxInfo.grContext()) {
            (*test)(reporter, ctxInfo);
            ctxInfo.grContext()->flush();
        }
    }
#endif
}
} // namespace skiatest

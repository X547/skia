/*
 * Copyright 2019 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/**************************************************************************************************
 *** This file was autogenerated from GrMixerEffect.fp; do not modify.
 **************************************************************************************************/
#include "GrMixerEffect.h"

#include "src/gpu/GrTexture.h"
#include "src/gpu/glsl/GrGLSLFragmentProcessor.h"
#include "src/gpu/glsl/GrGLSLFragmentShaderBuilder.h"
#include "src/gpu/glsl/GrGLSLProgramBuilder.h"
#include "src/sksl/SkSLCPP.h"
#include "src/sksl/SkSLUtil.h"
class GrGLSLMixerEffect : public GrGLSLFragmentProcessor {
public:
    GrGLSLMixerEffect() {}
    void emitCode(EmitArgs& args) override {
        GrGLSLFPFragmentBuilder* fragBuilder = args.fFragBuilder;
        const GrMixerEffect& _outer = args.fFp.cast<GrMixerEffect>();
        (void)_outer;
        auto weight = _outer.weight;
        (void)weight;
        weightVar = args.fUniformHandler->addUniform(&_outer, kFragment_GrShaderFlag,
                                                     kHalf_GrSLType, "weight");
        SkString _input1335(args.fInputColor);
        SkString _sample1335;
        if (_outer.inputFP_index >= 0) {
            _sample1335 = this->invokeChild(_outer.inputFP_index, _input1335.c_str(), args);
        } else {
            _sample1335.swap(_input1335);
        }
        fragBuilder->codeAppendf(
                R"SkSL(half4 inColor = %s;)SkSL", _sample1335.c_str());
        SkString _input1386("inColor");
        SkString _sample1386;
        _sample1386 = this->invokeChild(_outer.fp0_index, _input1386.c_str(), args);
        SkString _input1408("inColor");
        SkString _sample1408;
        if (_outer.fp1_index >= 0) {
            _sample1408 = this->invokeChild(_outer.fp1_index, _input1408.c_str(), args);
        } else {
            _sample1408.swap(_input1408);
        }
        fragBuilder->codeAppendf(
                R"SkSL(
%s = mix(%s, %s, %s);
)SkSL",
                args.fOutputColor, _sample1386.c_str(), _sample1408.c_str(),
                args.fUniformHandler->getUniformCStr(weightVar));
    }

private:
    void onSetData(const GrGLSLProgramDataManager& pdman,
                   const GrFragmentProcessor& _proc) override {
        const GrMixerEffect& _outer = _proc.cast<GrMixerEffect>();
        { pdman.set1f(weightVar, (_outer.weight)); }
    }
    UniformHandle weightVar;
};
GrGLSLFragmentProcessor* GrMixerEffect::onCreateGLSLInstance() const {
    return new GrGLSLMixerEffect();
}
void GrMixerEffect::onGetGLSLProcessorKey(const GrShaderCaps& caps,
                                          GrProcessorKeyBuilder* b) const {}
bool GrMixerEffect::onIsEqual(const GrFragmentProcessor& other) const {
    const GrMixerEffect& that = other.cast<GrMixerEffect>();
    (void)that;
    if (weight != that.weight) return false;
    return true;
}
GrMixerEffect::GrMixerEffect(const GrMixerEffect& src)
        : INHERITED(kGrMixerEffect_ClassID, src.optimizationFlags()), weight(src.weight) {
    if (src.inputFP_index >= 0) {
        inputFP_index = this->cloneAndRegisterChildProcessor(src.childProcessor(src.inputFP_index));
    }
    { fp0_index = this->cloneAndRegisterChildProcessor(src.childProcessor(src.fp0_index)); }
    if (src.fp1_index >= 0) {
        fp1_index = this->cloneAndRegisterChildProcessor(src.childProcessor(src.fp1_index));
    }
}
std::unique_ptr<GrFragmentProcessor> GrMixerEffect::clone() const {
    return std::unique_ptr<GrFragmentProcessor>(new GrMixerEffect(*this));
}

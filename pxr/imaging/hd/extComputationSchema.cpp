//
// Copyright 2023 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
////////////////////////////////////////////////////////////////////////

/* ************************************************************************** */
/* **                                                                      ** */
/* ** This file is generated by a script.                                  ** */
/* **                                                                      ** */
/* ** Do not edit it directly (unless it is within a CUSTOM CODE section)! ** */
/* ** Edit hdSchemaDefs.py instead to make changes.                        ** */
/* **                                                                      ** */
/* ************************************************************************** */

#include "pxr/imaging/hd/extComputationSchema.h"

#include "pxr/imaging/hd/retainedDataSource.h"

#include "pxr/base/trace/trace.h"

// --(BEGIN CUSTOM CODE: Includes)--
// --(END CUSTOM CODE: Includes)--

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdExtComputationSchemaTokens,
    HD_EXT_COMPUTATION_SCHEMA_TOKENS);

// --(BEGIN CUSTOM CODE: Schema Methods)--
// --(END CUSTOM CODE: Schema Methods)--

HdContainerDataSourceHandle
HdExtComputationSchema::GetInputValues() const
{
    return _GetTypedDataSource<HdContainerDataSource>(
        HdExtComputationSchemaTokens->inputValues);
}

HdVectorDataSourceHandle
HdExtComputationSchema::GetInputComputations() const
{
    return _GetTypedDataSource<HdVectorDataSource>(
        HdExtComputationSchemaTokens->inputComputations);
}

HdVectorDataSourceHandle
HdExtComputationSchema::GetOutputs() const
{
    return _GetTypedDataSource<HdVectorDataSource>(
        HdExtComputationSchemaTokens->outputs);
}

HdStringDataSourceHandle
HdExtComputationSchema::GetGlslKernel() const
{
    return _GetTypedDataSource<HdStringDataSource>(
        HdExtComputationSchemaTokens->glslKernel);
}

HdDataSourceBaseHandle
HdExtComputationSchema::GetCpuCallback() const
{
    return _GetTypedDataSource<HdDataSourceBase>(
        HdExtComputationSchemaTokens->cpuCallback);
}

HdSizetDataSourceHandle
HdExtComputationSchema::GetDispatchCount() const
{
    return _GetTypedDataSource<HdSizetDataSource>(
        HdExtComputationSchemaTokens->dispatchCount);
}

HdSizetDataSourceHandle
HdExtComputationSchema::GetElementCount() const
{
    return _GetTypedDataSource<HdSizetDataSource>(
        HdExtComputationSchemaTokens->elementCount);
}

/*static*/
HdContainerDataSourceHandle
HdExtComputationSchema::BuildRetained(
        const HdContainerDataSourceHandle &inputValues,
        const HdVectorDataSourceHandle &inputComputations,
        const HdVectorDataSourceHandle &outputs,
        const HdStringDataSourceHandle &glslKernel,
        const HdDataSourceBaseHandle &cpuCallback,
        const HdSizetDataSourceHandle &dispatchCount,
        const HdSizetDataSourceHandle &elementCount
)
{
    TfToken _names[7];
    HdDataSourceBaseHandle _values[7];

    size_t _count = 0;

    if (inputValues) {
        _names[_count] = HdExtComputationSchemaTokens->inputValues;
        _values[_count++] = inputValues;
    }

    if (inputComputations) {
        _names[_count] = HdExtComputationSchemaTokens->inputComputations;
        _values[_count++] = inputComputations;
    }

    if (outputs) {
        _names[_count] = HdExtComputationSchemaTokens->outputs;
        _values[_count++] = outputs;
    }

    if (glslKernel) {
        _names[_count] = HdExtComputationSchemaTokens->glslKernel;
        _values[_count++] = glslKernel;
    }

    if (cpuCallback) {
        _names[_count] = HdExtComputationSchemaTokens->cpuCallback;
        _values[_count++] = cpuCallback;
    }

    if (dispatchCount) {
        _names[_count] = HdExtComputationSchemaTokens->dispatchCount;
        _values[_count++] = dispatchCount;
    }

    if (elementCount) {
        _names[_count] = HdExtComputationSchemaTokens->elementCount;
        _values[_count++] = elementCount;
    }
    return HdRetainedContainerDataSource::New(_count, _names, _values);
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetInputValues(
    const HdContainerDataSourceHandle &inputValues)
{
    _inputValues = inputValues;
    return *this;
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetInputComputations(
    const HdVectorDataSourceHandle &inputComputations)
{
    _inputComputations = inputComputations;
    return *this;
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetOutputs(
    const HdVectorDataSourceHandle &outputs)
{
    _outputs = outputs;
    return *this;
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetGlslKernel(
    const HdStringDataSourceHandle &glslKernel)
{
    _glslKernel = glslKernel;
    return *this;
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetCpuCallback(
    const HdDataSourceBaseHandle &cpuCallback)
{
    _cpuCallback = cpuCallback;
    return *this;
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetDispatchCount(
    const HdSizetDataSourceHandle &dispatchCount)
{
    _dispatchCount = dispatchCount;
    return *this;
}

HdExtComputationSchema::Builder &
HdExtComputationSchema::Builder::SetElementCount(
    const HdSizetDataSourceHandle &elementCount)
{
    _elementCount = elementCount;
    return *this;
}

HdContainerDataSourceHandle
HdExtComputationSchema::Builder::Build()
{
    return HdExtComputationSchema::BuildRetained(
        _inputValues,
        _inputComputations,
        _outputs,
        _glslKernel,
        _cpuCallback,
        _dispatchCount,
        _elementCount
    );
}

/*static*/
HdExtComputationSchema
HdExtComputationSchema::GetFromParent(
        const HdContainerDataSourceHandle &fromParentContainer)
{
    return HdExtComputationSchema(
        fromParentContainer
        ? HdContainerDataSource::Cast(fromParentContainer->Get(
                HdExtComputationSchemaTokens->extComputation))
        : nullptr);
}

/*static*/
const TfToken &
HdExtComputationSchema::GetSchemaToken()
{
    return HdExtComputationSchemaTokens->extComputation;
}

/*static*/
const HdDataSourceLocator &
HdExtComputationSchema::GetDefaultLocator()
{
    static const HdDataSourceLocator locator(GetSchemaToken());
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetInputValuesLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->inputValues);
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetInputComputationsLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->inputComputations);
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetOutputsLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->outputs);
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetGlslKernelLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->glslKernel);
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetCpuCallbackLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->cpuCallback);
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetDispatchCountLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->dispatchCount);
    return locator;
}

/* static */
const HdDataSourceLocator &
HdExtComputationSchema::GetElementCountLocator()
{
    static const HdDataSourceLocator locator =
        GetDefaultLocator().Append(
            HdExtComputationSchemaTokens->elementCount);
    return locator;
} 

PXR_NAMESPACE_CLOSE_SCOPE
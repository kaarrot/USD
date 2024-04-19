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

#include "pxr/imaging/hd/cubeSchema.h"

#include "pxr/imaging/hd/retainedDataSource.h"

#include "pxr/base/trace/trace.h"

// --(BEGIN CUSTOM CODE: Includes)--
// --(END CUSTOM CODE: Includes)--

PXR_NAMESPACE_OPEN_SCOPE

TF_DEFINE_PUBLIC_TOKENS(HdCubeSchemaTokens,
    HD_CUBE_SCHEMA_TOKENS);

// --(BEGIN CUSTOM CODE: Schema Methods)--
// --(END CUSTOM CODE: Schema Methods)--

HdDoubleDataSourceHandle
HdCubeSchema::GetSize() const
{
    return _GetTypedDataSource<HdDoubleDataSource>(
        HdCubeSchemaTokens->size);
}

/*static*/
HdContainerDataSourceHandle
HdCubeSchema::BuildRetained(
        const HdDoubleDataSourceHandle &size
)
{
    TfToken _names[1];
    HdDataSourceBaseHandle _values[1];

    size_t _count = 0;

    if (size) {
        _names[_count] = HdCubeSchemaTokens->size;
        _values[_count++] = size;
    }
    return HdRetainedContainerDataSource::New(_count, _names, _values);
}

HdCubeSchema::Builder &
HdCubeSchema::Builder::SetSize(
    const HdDoubleDataSourceHandle &size)
{
    _size = size;
    return *this;
}

HdContainerDataSourceHandle
HdCubeSchema::Builder::Build()
{
    return HdCubeSchema::BuildRetained(
        _size
    );
}

/*static*/
HdCubeSchema
HdCubeSchema::GetFromParent(
        const HdContainerDataSourceHandle &fromParentContainer)
{
    return HdCubeSchema(
        fromParentContainer
        ? HdContainerDataSource::Cast(fromParentContainer->Get(
                HdCubeSchemaTokens->cube))
        : nullptr);
}

/*static*/
const TfToken &
HdCubeSchema::GetSchemaToken()
{
    return HdCubeSchemaTokens->cube;
}

/*static*/
const HdDataSourceLocator &
HdCubeSchema::GetDefaultLocator()
{
    static const HdDataSourceLocator locator(GetSchemaToken());
    return locator;
} 

PXR_NAMESPACE_CLOSE_SCOPE
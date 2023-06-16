//
// Copyright 2021 Pixar
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
#ifndef PXR_IMAGING_HD_FLATTENING_SCENE_H
#define PXR_IMAGING_HD_FLATTENING_SCENE_H

#include "pxr/imaging/hd/api.h"

#include "pxr/imaging/hd/dataSourceTypeDefs.h"
#include "pxr/imaging/hd/filteringSceneIndex.h"
#include "pxr/imaging/hd/flattenedPrimvarsDataSource.h"

#include "pxr/usd/sdf/pathTable.h"
#include <tbb/concurrent_unordered_map.h>

PXR_NAMESPACE_OPEN_SCOPE

TF_DECLARE_REF_PTRS(HdFlatteningSceneIndex);

///
/// \class HdFlatteningSceneIndex
///
/// A scene index that observes an input scene index and produces a comparable
/// scene in which inherited state is represented at leaf prims.
///
/// This kind of representation is typically required for legacy-style 
/// render delegates that require all the information to be available at the
/// leaf prims.
///
class HdFlatteningSceneIndex : public HdSingleInputFilteringSceneIndexBase
{
public:

    /// Creates a new flattening scene index.
    ///
    static HdFlatteningSceneIndexRefPtr New(
                HdSceneIndexBaseRefPtr const &inputScene,
                HdContainerDataSourceHandle const &inputArgs = nullptr) {
        return TfCreateRefPtr(
            new HdFlatteningSceneIndex(inputScene, inputArgs));
    }

    HD_API
    ~HdFlatteningSceneIndex() override;

    // satisfying HdSceneIndexBase
    HD_API 
    HdSceneIndexPrim GetPrim(const SdfPath &primPath) const override;

    HD_API 
    SdfPathVector GetChildPrimPaths(const SdfPath &primPath) const override;

protected:

    HD_API
    HdFlatteningSceneIndex(
        HdSceneIndexBaseRefPtr const &inputScene,
        HdContainerDataSourceHandle const &inputArgs);

    // satisfying HdSingleInputFilteringSceneIndexBase
    void _PrimsAdded(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::AddedPrimEntries &entries) override;

    void _PrimsRemoved(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::RemovedPrimEntries &entries) override;

    void _PrimsDirtied(
            const HdSceneIndexBase &sender,
            const HdSceneIndexObserver::DirtiedPrimEntries &entries) override;

    // Consolidate _recentPrims into _prims.
    void _ConsolidateRecentPrims();

private:
    // members
    using _PrimTable = SdfPathTable<HdSceneIndexPrim>;
    _PrimTable _prims;

    struct _PathHashCompare {
        static bool equal(const SdfPath& a, const SdfPath& b) {
            return a == b;
        }
        static size_t hash(const SdfPath& path) {
            return hash_value(path);
        }
    };
    using _RecentPrimTable =
        tbb::concurrent_hash_map<SdfPath, HdSceneIndexPrim, _PathHashCompare>;
    mutable _RecentPrimTable _recentPrims;

    const bool _flattenXform;
    const bool _flattenVisibility;
    const bool _flattenPurpose;
    const bool _flattenModel;
    const bool _flattenMaterialBindings;
    const bool _flattenPrimvars;
    TfTokenVector _dataSourceNames;

    const HdContainerDataSourceHandle _identityXform;
    const HdContainerDataSourceHandle _identityVis;
    const HdContainerDataSourceHandle _identityPurpose;
    const HdTokenDataSourceHandle _identityDrawMode;

    // methods
    void _DirtyHierarchy(
        const SdfPath &primPath,
        const HdDataSourceLocatorSet &dirtyLocators,
        HdSceneIndexObserver::DirtiedPrimEntries *dirtyEntries);

    // ------------------------------------------------------------------------

    /// wraps the input scene's prim-level data sources in order to deliver
    /// overriden value
    class _PrimLevelWrappingDataSource : public HdContainerDataSource
    {
    public:
        HD_DECLARE_DATASOURCE(_PrimLevelWrappingDataSource);

        _PrimLevelWrappingDataSource(
                const HdFlatteningSceneIndex &scene,
                const SdfPath &primPath,
                HdContainerDataSourceHandle inputDataSource);

        void UpdateInputDataSource(HdContainerDataSourceHandle inputDataSource);

        bool PrimDirtied(const HdDataSourceLocatorSet &locators);

        TfTokenVector GetNames() override;
        HdDataSourceBaseHandle Get(const TfToken &name) override;

    private:
        HdContainerDataSourceHandle _GetParentPrimDataSource() const;

        HdDataSourceBaseHandle _GetXform();
        HdDataSourceBaseHandle _GetVis();
        HdDataSourceBaseHandle _GetPurpose();
        HdDataSourceBaseHandle _GetModel();
        HdDataSourceBaseHandle _GetDrawMode(
            const HdContainerDataSourceHandle &model);
        HdTokenDataSourceHandle _GetDrawModeUncached(
            const HdContainerDataSourceHandle &model);
        HdDataSourceBaseHandle _GetMaterialBindings();
        HdContainerDataSourceHandle _GetMaterialBindingsUncached();
        HdFlattenedPrimvarsDataSourceHandle _GetPrimvars();
        HdFlattenedPrimvarsDataSourceHandle _GetPrimvarsUncached();

        const HdFlatteningSceneIndex &_sceneIndex;
        SdfPath _primPath;
        HdContainerDataSourceHandle _inputDataSource;
        HdContainerDataSourceAtomicHandle _computedXformDataSource;
        HdContainerDataSourceAtomicHandle _computedVisDataSource;
        HdContainerDataSourceAtomicHandle _computedPurposeDataSource;
        HdTokenDataSource::AtomicHandle _computedDrawModeDataSource;
        // Stored as a base rather than a container so we can use cast to
        // distinguish between a cached value and the absence of a cached value.
        // Internally, this is set to a retained bool=false data source to 
        // indicate that no binding is present.
        HdDataSourceBaseAtomicHandle _computedMaterialBindingsDataSource;
        HdFlattenedPrimvarsDataSource::AtomicHandle _computedPrimvarsDataSource;
    };

    HD_DECLARE_DATASOURCE_HANDLES(_PrimLevelWrappingDataSource);

};

PXR_NAMESPACE_CLOSE_SCOPE

#endif

//
// Copyright 2016 Pixar
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
#ifndef PXR_IMAGING_HD_ST_RESOURCE_BINDER_H
#define PXR_IMAGING_HD_ST_RESOURCE_BINDER_H

#include "pxr/pxr.h"
#include "pxr/imaging/hdSt/api.h"
#include "pxr/imaging/hd/version.h"

#include "pxr/imaging/hd/binding.h"
#include "pxr/imaging/hgi/shaderProgram.h"
#include "pxr/base/tf/token.h"
#include "pxr/base/tf/stl.h"
#include "pxr/base/tf/staticTokens.h"

#include <memory>

PXR_NAMESPACE_OPEN_SCOPE


class HdStDrawItem;

using HdStBufferResourceSharedPtr = 
    std::shared_ptr<class HdStBufferResource>;
using HdStBufferArrayRangeSharedPtr =
    std::shared_ptr<class HdStBufferArrayRange>;

using HdStShaderCodeSharedPtr = std::shared_ptr<class HdStShaderCode>;
using HdStShaderCodeSharedPtrVector = std::vector<HdStShaderCodeSharedPtr>;
using HdBindingRequestVector = std::vector<class HdBindingRequest>;

/// Suffixes appended to material param names for a binding name.
///
#define HDST_RESOURCE_BINDING_SUFFIX_TOKENS     \
    ((fallback, "_fallback"))                   \
    ((samplingTransform, "_samplingTransform")) \
    ((layout, "_layout"))                       \
    ((texture, "_texture"))                     \
    ((valid, "_valid"))

TF_DECLARE_PUBLIC_TOKENS(HdSt_ResourceBindingSuffixTokens,
                         HDST_RESOURCE_BINDING_SUFFIX_TOKENS);

/// \class HdSt_ResourceBinder
/// 
/// A helper class to maintain all vertex/buffer/uniform binding points to be
/// used for both codegen time and rendering time.
///
/// Storm uses 6 different types of coherent buffers.
///
/// 1. Constant buffer
///   constant primvars, which is uniform for all instances/elements/vertices.
///     ex. transform, object color
//    [SSBO, BindlessUniform]
///
/// 2. Instance buffer
///   instance primvars, one-per-instance.
///     ex. translate/scale/rotate, instanceIndices
//    [SSBO, BindlessUniform]
///
/// 3. Element buffer
///   element primvars. one-per-element (face, line).
///     ex. face color
///   [SSBO]
///
/// 4. Vertex buffer
///   vertex primvars. one-per-vertex.
///     ex. positions, normals, vertex color
///   [VertexAttribute]
///
/// 5. Index buffer
///   points/triangles/quads/lines/patches indices.
///     ex. indices, primitive param.
///   [IndexAttribute, SSBO]
///
/// 6. DrawIndex buffer
///   draw command data. one-per-drawitem (gl_DrawID equivalent)
///     ex. drawing coordinate, instance counts
///   [VertexAttribute]
///
///
///
/// For instance index indirection, three bindings are needed:
///
///    +-----------------------------------------+
///    |  instance indices buffer resource       | <-- <arrayBinding>
///    +-----------------------------------------+
///    |* culled instance indices buffer resource| <-- <culledArrayBinding>
///    +-----------------------------------------+  (bindless uniform
///                  ^   ^      ^                      or SSBO)
/// DrawCalls +---+  |   |      |
///          0|   |---   |      |
///           +---+      |      |
///          1|   |-------      |
///           +---+             |
///          2|   |--------------
///           +---+
///             ^
///             --- <baseBinding>
///                  (immediate:uniform, indirect:vertex attrib)
///
/// (*) GPU frustum culling shader shuffles instance indices into
///     culled indices buffer.
///
///
/// HdSt_ResourceBinder also takes custom bindings.
///
/// Custom bindings are used to manage bindable resources for
/// glsl shader code which is not itself generated by codegen.
///
/// For each custom binding, codegen will emit a binding definition
/// that can be used as the value of a glsl \a binding or
/// \a location layout qualifier.
///
/// e.g. Adding a custom binding of 2 for "paramsBuffer", will
/// cause codegen to emit the definition:
/// \code
/// #define paramsBuffer_Binding 2
/// \endcode
/// which can be used in a custom glsl resource declaration as:
/// \code
/// layout (binding = paramsBuffer_Binding) buffer ParamsBuffer { ... };
/// \endcode
///
class HdSt_ResourceBinder {
public:
    /// binding metadata for codegen
    class MetaData {
    public:
        MetaData() : instancerNumLevels(0) {}

        typedef size_t ID;
        /// Returns the hash value of this metadata.
        HDST_API
        ID ComputeHash() const;

        // -------------------------------------------------------------------
        // for a primvar in interleaved buffer array (Constant, ShaderData)
        struct StructEntry {
            StructEntry(TfToken const &name,
                        TfToken const &dataType,
                        int offset, int arraySize)
                : name(name)
                , dataType(dataType)
                , offset(offset)
                , arraySize(arraySize)
            {}

            TfToken name;
            TfToken dataType;
            int offset;
            int arraySize;

            bool operator < (StructEntry const &other) const {
                return offset < other.offset;
            }
        };
        struct StructBlock {
            StructBlock(TfToken const &name)
                : blockName(name) {}
            TfToken blockName;
            std::vector<StructEntry> entries;
        };
        typedef std::map<HdBinding, StructBlock> StructBlockBinding;

        // -------------------------------------------------------------------
        // for a primvar in non-interleaved buffer array (Vertex, Element, ...)
        struct Primvar {
            Primvar() {}
            Primvar(TfToken const &name, TfToken const &dataType)
                : name(name), dataType(dataType) {}
            TfToken name;
            TfToken dataType;
        };
        typedef std::map<HdBinding, Primvar> PrimvarBinding;

        // -------------------------------------------------------------------
        // for instance primvars
        struct NestedPrimvar {
            NestedPrimvar() {}
            NestedPrimvar(TfToken const &name, TfToken const &dataType,
                        int level)
                : name(name), dataType(dataType), level(level) {}
            TfToken name;
            TfToken dataType;
            int level;
        };
        typedef std::map<HdBinding, NestedPrimvar> NestedPrimvarBinding;

        // -------------------------------------------------------------------
        // for shader parameter accessors
        struct ShaderParameterAccessor {
             ShaderParameterAccessor() {}
             ShaderParameterAccessor(TfToken const &name,
                                     TfToken const &dataType,
                                     std::string const &swizzle=std::string(),
                                     TfTokenVector const &inPrimvars=TfTokenVector(),
                                     bool const isPremultiplied=false,
                                     bool const processTextureFallbackValue=false)
                 : name(name), dataType(dataType), swizzle(swizzle),
                  inPrimvars(inPrimvars), isPremultiplied(isPremultiplied),
                  processTextureFallbackValue(processTextureFallbackValue) {}
             TfToken name;        // e.g. Kd
             TfToken dataType;    // e.g. vec4
             std::string swizzle; // e.g. xyzw
             TfTokenVector inPrimvars; // for primvar renaming and texture
                                       // coordinates,
             bool isPremultiplied; // indicates if texture parameter has been 
                                   // pre-multiplied by alpha on the CPU
             bool processTextureFallbackValue; // use NAME_fallback from shader
                                               // bar if texture is not valid
                                               // (determineed from bool
                                               // NAME_valid or bindless
                                               // handle), only supported for
                                               // material shader and for uv
                                               // and field textures.
        };
        typedef std::map<HdBinding, ShaderParameterAccessor> ShaderParameterBinding;

        // -------------------------------------------------------------------
        // for specific buffer array (drawing coordinate, instance indices)
        struct BindingDeclaration {
            BindingDeclaration() {}
            BindingDeclaration(TfToken const &name,
                         TfToken const &dataType,
                         HdBinding binding)
                : name(name), dataType(dataType), binding(binding) {}
            TfToken name;
            TfToken dataType;
            HdBinding binding;
        };

        // -------------------------------------------------------------------

        StructBlockBinding constantData;
        StructBlockBinding shaderData;
        StructBlockBinding topologyVisibilityData;
        PrimvarBinding elementData;
        PrimvarBinding vertexData;
        PrimvarBinding fvarData;
        PrimvarBinding computeReadWriteData;
        PrimvarBinding computeReadOnlyData;
        NestedPrimvarBinding instanceData;
        int instancerNumLevels;

        ShaderParameterBinding shaderParameterBinding;

        BindingDeclaration drawingCoord0Binding;
        BindingDeclaration drawingCoord1Binding;
        BindingDeclaration drawingCoord2Binding;
        BindingDeclaration drawingCoordIBinding;
        BindingDeclaration instanceIndexArrayBinding;
        BindingDeclaration culledInstanceIndexArrayBinding;
        BindingDeclaration instanceIndexBaseBinding;
        BindingDeclaration primitiveParamBinding;
        BindingDeclaration edgeIndexBinding;

        StructBlockBinding customInterleavedBindings;
        std::vector<BindingDeclaration> customBindings;
    };

    /// Constructor.
    HDST_API
    HdSt_ResourceBinder();

    /// Assign all binding points used in drawitem and custom bindings.
    /// Returns metadata to be used for codegen.
    HDST_API
    void ResolveBindings(HdStDrawItem const *drawItem,
                         HdStShaderCodeSharedPtrVector const &shaders,
                         MetaData *metaDataOut,
                         bool indirect,
                         bool instanceDraw,
                         HdBindingRequestVector const &customBindings);

    /// Assign all binding points used in computation.
    /// Returns metadata to be used for codegen.
    HDST_API
    void ResolveComputeBindings(HdBufferSpecVector const &readWriteBufferSpecs,
                                HdBufferSpecVector const &readOnlyBufferSpecs,
                                HdStShaderCodeSharedPtrVector const &shaders,
                                MetaData *metaDataOut);
    
    /// call GL introspection APIs and fix up binding locations,
    /// in case if explicit resource location qualifier is not available
    /// (GL 4.2 or before)
    HDST_API
    void IntrospectBindings(HgiShaderProgramHandle const & hgiProgram);

    HDST_API
    void Bind(HdBindingRequest const& req) const;
    HDST_API
    void Unbind(HdBindingRequest const& req) const;

    /// bind/unbind BufferArray
    HDST_API
    void BindBufferArray(HdStBufferArrayRangeSharedPtr const &bar) const;
    HDST_API
    void UnbindBufferArray(HdStBufferArrayRangeSharedPtr const &bar) const;

    /// bind/unbind interleaved constant buffer
    HDST_API
    void BindConstantBuffer(
        HdStBufferArrayRangeSharedPtr const & constantBar) const;
    HDST_API
    void UnbindConstantBuffer(
        HdStBufferArrayRangeSharedPtr const &constantBar) const;

    /// bind/unbind interleaved buffer
    HDST_API
    void BindInterleavedBuffer(
        HdStBufferArrayRangeSharedPtr const & constantBar,
        TfToken const &name) const;
    HDST_API
    void UnbindInterleavedBuffer(
        HdStBufferArrayRangeSharedPtr const &constantBar,
        TfToken const &name) const;

    /// bind/unbind nested instance BufferArray
    HDST_API
    void BindInstanceBufferArray(
        HdStBufferArrayRangeSharedPtr const &bar, int level) const;
    HDST_API
    void UnbindInstanceBufferArray(
        HdStBufferArrayRangeSharedPtr const &bar, int level) const;

    /// bind/unbind shader parameters and textures
    HDST_API
    void BindShaderResources(HdStShaderCode const *shader) const;
    HDST_API
    void UnbindShaderResources(HdStShaderCode const *shader) const;

    /// piecewise buffer binding utility
    /// (to be used for frustum culling, draw indirect result)
    HDST_API
    void BindBuffer(TfToken const &name,
                    HdStBufferResourceSharedPtr const &resource) const;
    HDST_API
    void BindBuffer(TfToken const &name,
                    HdStBufferResourceSharedPtr const &resource,
                    int offset, int level=-1) const;
    HDST_API
    void UnbindBuffer(TfToken const &name,
                      HdStBufferResourceSharedPtr const &resource,
                      int level=-1) const;

    /// bind(update) a standalone uniform (unsigned int)
    HDST_API
    void BindUniformui(TfToken const &name, int count,
                       const unsigned int *value) const;

    /// bind a standalone uniform (signed int, ivec2, ivec3, ivec4)
    HDST_API
    void BindUniformi(TfToken const &name, int count, const int *value) const;

    /// bind a standalone uniform array (int[N])
    HDST_API
    void BindUniformArrayi(TfToken const &name, int count, const int *value) const;

    /// bind a standalone uniform (float, vec2, vec3, vec4, mat4)
    HDST_API
    void BindUniformf(TfToken const &name, int count, const float *value) const;

    /// Returns binding point.
    /// XXX: exposed temporarily for drawIndirectResult
    /// see Hd_IndirectDrawBatch::_BeginGPUCountVisibleInstances()
    HdBinding GetBinding(TfToken const &name, int level=-1) const {
        HdBinding binding;
        TfMapLookup(_bindingMap, NameAndLevel(name, level), &binding);
        return binding;
    }

    int GetNumReservedUniformBlockLocations() const {
        return _numReservedUniformBlockLocations;
    }

    int GetNumReservedTextureUnits() const {
        return _numReservedTextureUnits;
    }

private:
    // for batch execution
    struct NameAndLevel {
        NameAndLevel(TfToken const &n, int lv=-1) :
            name(n), level(lv) {}
        TfToken name;
        int level;

        bool operator < (NameAndLevel const &other) const {
            return name  < other.name ||
                  (name == other.name && level < other.level);
        }
    };
    typedef std::map<NameAndLevel, HdBinding> _BindingMap;
    _BindingMap _bindingMap;
    int _numReservedUniformBlockLocations;
    int _numReservedTextureUnits;
};


PXR_NAMESPACE_CLOSE_SCOPE

#endif  // PXR_IMAGING_HD_ST_RESOURCE_BINDER_H

source ~/venvs/usd_3.11/bin/activate

python ./build_scripts/build_usd.py --no-usdview --no-materialx --no-imaging --generator Ninja ./install_prefix



cd /data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/build/OpenUSD

cmake -DCMAKE_INSTALL_PREFIX="/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix" -DCMAKE_PREFIX_PATH="/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix" -DCMAKE_BUILD_TYPE=Debug  -G "Ninja"  -DPXR_PREFER_SAFETY_OVER_SPEED=ON -DPXR_ENABLE_PYTHON_SUPPORT=ON -DPXR_USE_DEBUG_PYTHON=OFF -DPython3_EXECUTABLE="/data/data/com.termux/files/home/venvs/usd_3.11/bin/python" -DPython3_LIBRARY="/data/data/com.termux/files/usr/lib/libpython3.11.so" -DPython3_INCLUDE_DIR="/data/data/com.termux/files/usr/include/python3.11" -DBUILD_SHARED_LIBS=ON -DTBB_USE_DEBUG_BUILD=OFF -DPXR_BUILD_DOCUMENTATION=OFF -DPXR_BUILD_HTML_DOCUMENTATION=OFF -DPXR_BUILD_PYTHON_DOCUMENTATION=OFF -DPXR_BUILD_TESTS=ON -DPXR_BUILD_EXAMPLES=OFF -DPXR_BUILD_TUTORIALS=OFF -DPXR_BUILD_USD_TOOLS=OFF -DPXR_BUILD_IMAGING=OFF -DPXR_BUILD_USD_IMAGING=OFF -DPXR_BUILD_USDVIEW=OFF -DPXR_BUILD_ALEMBIC_PLUGIN=OFF -DPXR_BUILD_DRACO_PLUGIN=OFF -DPXR_ENABLE_MATERIALX_SUPPORT=OFF -DPXR_BUILD_MAYAPY_TESTS=OFF -DPXR_BUILD_ANIMX_TESTS=OFF -DBoost_NO_BOOST_CMAKE=On -DBoost_NO_SYSTEM_PATHS=True "/data/data/com.termux/files/home/SRC/OpenUSD"


ninja -j 2

export PYTHONPATH=/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/lib/python:$PYTHONPATH

export LD_LIBRARY_PATH=/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/lib:/data/data/com.termux/files/home/SRC/oneTBB/install_prefix/lib


---- Boost change 1.82.0
/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/include/boost/python/instance_holder.hpp

-   virtual ~instance_holder();
+    virtual ~instance_holder() noexcept;

/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/include/boost/python/object/value_holder.hpp

struct value_holder : instance_holder

+    virtual ~value_holder() noexcept {}



# testing

cd build
make install
cmake -R Debug -V

VERBOSE=1 make testArchAbi

- without make install- testing first test
/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/build/pxr/base/arch/testArchAbi

# wrapBBoxCache python binding - 82% build

/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/include/boost/python/instance_holder.hpp
    
struct BOOST_PYTHON_DECL instance_holder : private noncopyable
{
 public:
    instance_holder();
    virtual ~instance_holder() noexcept;
    
    // return the next holder in a chain
>    instance_holder* next() const;

    

/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/include/boost/python/object/value_holder.hpp

#  define BOOST_PP_ITERATION_PARAMS_1 (4, (0, BOOST_PYTHON_MAX_ARITY, <boost/python/object/value_holder.hpp>, 1))
#  include BOOST_PP_ITERATE()

>    virtual ~value_holder() noexcept {return ;}
        



acheLineSize()                                Function: Arch_ValidateAssumptions                File: /data/data/com.termux/files/home/SRC/OpenUSD/pxr/base/arch/assumptions.cpp            Line: 155


# plugin symbols


'''
$ nm -gD ../tests/lib/libtestArchAbiPlugin.so
00000000000020e8 W _ZN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase2C2Ev
0000000000002188 W _ZN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase2D0Ev
0000000000002178 W _ZN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase2D2Ev
00000000000020ac W _ZN32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiEC2Ev
0000000000002130 W _ZN32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiED0Ev
000000000000210c W _ZN32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiED2Ev
0000000000002160 W _ZNK32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiE4nameEv
0000000000003280 V _ZTIN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase1E
0000000000003290 V _ZTIN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase2E
00000000000032a0 V _ZTIN32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiEE
0000000000000d98 V _ZTSN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase1E
0000000000000dcb V _ZTSN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase2E
0000000000000d60 V _ZTSN32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiEE
                 U _ZTVN10__cxxabiv117__class_type_infoE
                 U _ZTVN10__cxxabiv121__vmi_class_type_infoE
00000000000032d8 V _ZTVN32pxrInternal_v0_24__pxrReserved__12ArchAbiBase2E
0000000000003258 V _ZTVN32pxrInternal_v0_24__pxrReserved__14ArchAbiDerivedIiEE
                 U _ZdlPv
                 U _Znwm
                 U __cxa_atexit@LIBC
                 U __cxa_finalize@LIBC
                 U __cxa_pure_virtual
                 U __register_atfork@LIBC
0000000000002080 T newDerived
'''

----------

testArchDemangle

TfEnvSetting


testArOptionalImplementation_1


testPlug


testTsSplineAPI


TfEnvSetting


ctest -C Debug -V -R TfEnvSetting -V


less /data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/build/Testing/Failed-Diffs/2024-08-10T22.09.14/TfEnvSetting/debugTfEnvSettingStderr.baseline.txt

less /data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/build/Testing/Failed-Diffs/2024-08-10T22.09.14/TfEnvSetting/debugTfEnvSettingStderr.result.txt

ArchWarn: ARCH_CACHE_LINE_SIZE != Arch_ObtainCacheLineSize()
Function:Arch_ValidateAssumptions                                  File: /data/data/com.termux/files/home/SRC/OpenUSD/pxr/base/arch/assumptions.cpp
Line: 155


# testUsdModel

/data/data/com.termux/files/usr/bin/python "/data/data/com.termux/files/home/SRC/OpenUSD/cmake/macros/testWrapper.py" "--verbose" "--testenv-dir=/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/tests/ctest/testUsdModel" "--env-var=PYTHONPATH=/data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/lib/python:" "/data/data/com.termux/files/usr/bin/python /data/data/com.termux/files/home/SRC/OpenUSD/install_prefix/tests/testUsdModel"

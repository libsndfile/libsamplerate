layout: default
---

# Building for Android

An Android `gradle` project is located in the `android/` directory. The project
uses the standard [NDK CMake](https://developer.android.com/ndk/guides/cmake)
build system to generate a [prefab](https://google.github.io/prefab/) NDK package.

## Building the prefab package / .aar
The following commands will build `libsamplerate` as a prefab NDK package and place
it into an [.aar](https://developer.android.com/studio/projects/android-library) library.

You will need `gradle` version 8.7+ installed in in your path.
```
cd android/
gradle assembleRelease
```

The resulting `.aar` will be located at:
`android/build/outputs/aar/samplerate-release.aar`

If you need to specify additional arguments to the `cmake` build, change the
NDK version used for the build, etc, you can do so by editing the `gradle` build
script located at:

`android/build.gradle.kts`

## Using as a dependency
After building the `.aar`, do one of the following:
1. `gradle publishToMavenLocal` is already supported in the build script
2. `gradle publishToMavenRepository` is not setup, but you can edit `android/build.gradle.kts`
   to add your own maven repository to publish to
3. Copy the `.aar` directly to the `libs/` directory of your project (not recommended)

Then, add the library to your project's dependencies in your `build.gradle.kts`:
```
dependencies {
    implementation("com.meganerd:samplerate:0.2.2-android-rc1")
}
```

Update your `CMakeLists.txt` to find and link the prefab package, which will be
extracted from the `aar` by the build system:

```
find_package(samplerate REQUIRED CONFIG)

target_link_libraries(${CMAKE_PROJECT_NAME} samplerate::samplerate)
```

That's it! You can now `#include <samplerate.h>` in your NDK source code.

## Testing on a device
To run the tests, follow these steps:
1. Ensure `adb` is in your path.
2. Have a single device (or emulator) connected and in debug mode. The testing task
only supports a single device. If you have more than one connected (or none) it will
notify you with an error.
3. You will also need `bash` to run the test script

Run the following commands:
```
cd android/
gradle ndkTest
```

The test task `:ndkTest` will run `gradle clean assembleRelease` with the following
options set for testing:
* `-DBUILD_SHARED_LIBS=OFF`
* `-DBUILD_TESTING=ON`

Then it runs `android/ndk-test.sh`, which pushes the binaries located at 
`android/build/intermediates/cmake/release/obj/$ABI` to `/data/local/tmp/libsamplerate/test`
on the device, and uses `adb` to execute them. The results will be printed to the console.


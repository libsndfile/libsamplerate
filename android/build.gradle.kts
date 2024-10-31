@file:Suppress("UnstableApiUsage")

plugins {
    alias(libs.plugins.library)
    id("maven-publish")
}

// project.name ("samplerate") defined in settings.gradle.kts
project.group = "com.meganerd"
project.version = "0.2.2-android-rc1"

android {
    namespace = "${project.group}.${project.name}"
    compileSdk = libs.versions.compilesdk.get().toInt()

    defaultConfig {
        minSdk = libs.versions.minsdk.get().toInt()

        buildToolsVersion = libs.versions.buildtools.get()
        ndkVersion = libs.versions.ndk.get()
        ndk {
            abiFilters += listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")
        }
        externalNativeBuild {
            // build static libs and testing binaries only when running :ndkTest
            val buildSharedLibs = if (isTestBuild()) "OFF" else "ON"
            val buildTesting = if (isTestBuild()) "ON" else "OFF"

            cmake {
                cppFlags += "-std=c++17"
                arguments += "-DANDROID_STL=c++_shared"

                arguments += "-DBUILD_SHARED_LIBS=$buildSharedLibs"
                arguments += "-DBUILD_TESTING=$buildTesting"
                arguments += "-DLIBSAMPLERATE_INSTALL=OFF"
                arguments += "-DLIBSAMPLERATE_EXAMPLES=OFF"
            }
        }
    }

    externalNativeBuild {
        cmake {
            path = file("${project.projectDir.parentFile}/CMakeLists.txt")
            version = libs.versions.cmake.get()
        }
    }

    buildFeatures {
        prefabPublishing = true
    }

    prefab {
        create(project.name) {
            headers = "${project.projectDir.parentFile}/include"
        }
    }

    packaging {
        // avoids duplicating libs in .aar due to using prefab
        jniLibs {
            excludes += "**/*"
        }
    }
}

tasks.register<Exec>(getTestTaskName()) {
    commandLine("./ndk-test.sh")
}

tasks.register<Delete>("cleanCxxDir") {
    delete(".cxx")
}

publishing {
    repositories {
        mavenLocal()
    }

    publications {
        create<MavenPublication>(project.name) {
            artifact("${project.projectDir}/build/outputs/aar/${project.name}-release.aar")
            artifactId = "${project.name}-android"
        }
    }
}

afterEvaluate {
    tasks.named("assembleRelease") {
        mustRunAfter("clean")
    }
    tasks.named(getTestTaskName()) {
        dependsOn("clean", "assembleRelease")
    }

    tasks.named("publish${project.name.cap()}PublicationToMavenLocal").configure {
        dependsOn("clean", "assembleRelease")
    }

    // suggests running ":ndkTest" task instead of default testing tasks
    listOf(
        "check",
        "test",
        "testDebugUnitTest",
        "testReleaseUnitTest",
        "connectedCheck",
        "connectedAndroidTest",
        "connectedDebugAndroidTest",
    ).forEach {
        tasks.named(it) {
            doLast {
                println(":$it task not supported; use :${getTestTaskName()} to run tests via adb")
            }
        }
    }

    tasks.named("clean") {
        finalizedBy("cleanCxxDir")
    }
}

fun getTestTaskName(): String = "ndkTest"

fun isTestBuild(): Boolean = gradle.startParameter.taskNames.contains(getTestTaskName())

// capitalize the first letter to make task names matched when written in camel case
fun String.cap(): String = this.replaceFirstChar { it.uppercase() }

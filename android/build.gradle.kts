@file:Suppress("UnstableApiUsage")

val requiredGradleVersion = "8.9"

require(gradle.gradleVersion == requiredGradleVersion) {
    "Gradle version $requiredGradleVersion required (current version: ${gradle.gradleVersion})"
}

plugins {
    alias(libs.plugins.library)
    id("maven-publish")
}

// project.name ("samplerate") defined in settings.gradle.kts
project.group = "com.meganerd"
project.version = "0.2.2-android-r1"

val abis = listOf("armeabi-v7a", "arm64-v8a", "x86", "x86_64")

android {
    namespace = "${project.group}.${project.name}"
    compileSdk = libs.versions.compilesdk.get().toInt()

    defaultConfig {
        minSdk = libs.versions.minsdk.get().toInt()

        buildToolsVersion = libs.versions.buildtools.get()
        ndkVersion = libs.versions.ndk.get()
        ndk {
            abiFilters += abis
        }
        externalNativeBuild {
            // build static libs and testing binaries only when running :ndkTest
            val buildSharedLibs = if (isTestBuild()) "OFF" else "ON"
            val buildTesting = if (isTestBuild()) "ON" else "OFF"

            cmake {
                arguments += "-DANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES=ON"

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
}

tasks.register<Zip>("prefabAar") {
    archiveFileName = "${project.name}-release.aar"
    destinationDirectory = file("build/outputs/prefab-aar")

    from("aar-template")
    from("${projectDir.parentFile}/include") {
        include("**/*.h")
        into("prefab/modules/${project.name}/include")
    }
    abis.forEach { abi ->
        from("build/intermediates/cmake/release/obj/$abi") {
            include("lib${project.name}.so")
            into("prefab/modules/${project.name}/libs/android.$abi")
        }
    }
}

tasks.register<Exec>(getTestTaskName()) {
    commandLine("./ndk-test.sh")
}

tasks.named<Delete>("clean") {
    delete.add(".cxx")
}

afterEvaluate {
    tasks.named("preBuild") {
        mustRunAfter("clean")
    }

    tasks.named("prefabAar") {
        dependsOn("externalNativeBuildRelease")
    }

    tasks.named("generatePomFileFor${project.name.cap()}Publication") {
        mustRunAfter("prefabAar")
    }

    tasks.named("publish") {
        dependsOn("clean", "prefabAar")
    }

    tasks.named(getTestTaskName()) {
        dependsOn("clean", "externalNativeBuildRelease")
    }
}

publishing {
    val githubPackagesUrl = "https://maven.pkg.github.com/jg-hot/libsamplerate-android"

    repositories {
        maven {
            url = uri(githubPackagesUrl)
            credentials {
                username = properties["gpr.user"]?.toString()
                password = properties["gpr.key"]?.toString()
            }
        }
    }

    publications {
        create<MavenPublication>(project.name) {
            artifact("build/outputs/prefab-aar/${project.name}-release.aar")
            artifactId = "${project.name}-android"

            pom {
                distributionManagement {
                    downloadUrl = githubPackagesUrl
                }
            }
        }
    }
}

tasks.named<Wrapper>("wrapper") {
    gradleVersion = requiredGradleVersion
}

fun getTestTaskName(): String = "ndkTest"

fun isTestBuild(): Boolean = gradle.startParameter.taskNames.contains(getTestTaskName())

// capitalize the first letter to make task names matched when written in camel case
fun String.cap(): String = this.replaceFirstChar { it.uppercase() }

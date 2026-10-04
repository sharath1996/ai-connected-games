pluginManagement {
    repositories {
        google()
        mavenCentral()
        gradlePluginPortal()
    }
}

dependencyResolutionManagement {
    repositories {
        google()
        mavenCentral()
        // usb-serial-for-android is distributed via JitPack
        maven { url = uri("https://jitpack.io") }
    }
}

rootProject.name = "SmartClockAlarm"
include(":app")

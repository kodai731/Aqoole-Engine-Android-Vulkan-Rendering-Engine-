/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
this file is modified by Shigeoka Kodai
the original file is at https://github.com/android/ndk-samples/tree/main/teapots
 */
/*
 * Copyright 2022 Shigeoka Kodai
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.aqoole

import android.app.Application
import android.content.pm.PackageManager
import android.content.pm.ApplicationInfo
import android.util.Log
import android.widget.Toast

//import javax.microedition.khronos.opengles.GL10;
//import android.opengl.GLUtils;
class vulkanApplication : Application() {
    override fun onCreate() {
        super.onCreate()
        Log.w("native-activity", "onCreate")
        val pm = applicationContext.packageManager
        val ai: ApplicationInfo?
        ai = try {
            pm.getApplicationInfo(this.packageName, 0)
        } catch (e: PackageManager.NameNotFoundException) {
            null
        }
        val applicationName =
            (if (ai != null) pm.getApplicationLabel(ai) else "(unknown)") as String
        Toast.makeText(this, applicationName, Toast.LENGTH_SHORT).show()
    }
    init {
        System.loadLibrary("vulkanNativeActivity")
    }
}
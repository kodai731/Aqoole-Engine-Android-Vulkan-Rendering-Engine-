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
the original is at https://github.com/android/ndk-samples/tree/main/teapots (Apache 2.0 License)
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

import android.app.NativeActivity
import android.os.Bundle
import android.view.View.OnSystemUiVisibilityChangeListener
import android.annotation.TargetApi
import com.aqoole.vulkanNativeActivity
import android.widget.PopupWindow
import android.widget.TextView
import android.annotation.SuppressLint
import android.content.Intent
import android.net.Uri
import android.os.Build
import android.provider.DocumentsContract
import android.util.Log
import android.view.LayoutInflater
import com.aqoole.R
import android.view.WindowManager
import android.widget.LinearLayout
import android.view.ViewGroup.MarginLayoutParams
import android.view.Gravity
import android.view.View

class vulkanNativeActivity : NativeActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        //Hide toolbar
        val SDK_INT = Build.VERSION.SDK_INT
        if (SDK_INT >= 19) {
            setImmersiveSticky()
            val decorView = window.decorView
            decorView.setOnSystemUiVisibilityChangeListener { setImmersiveSticky() }
        }
        Log.i("aqoole kotlin", "class name = " + vulkanNativeActivity::javaClass.name);
    }

    @TargetApi(19)
    override fun onResume() {
        super.onResume()

        //Hide toolbar
        val SDK_INT = Build.VERSION.SDK_INT
        if (SDK_INT >= 11 && SDK_INT < 14) {
            window.decorView.systemUiVisibility = View.STATUS_BAR_HIDDEN
        } else if (SDK_INT >= 14 && SDK_INT < 19) {
            window.decorView.systemUiVisibility =
                View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_LOW_PROFILE
        } else if (SDK_INT >= 19) {
            setImmersiveSticky()
        }
    }

    // Our popup window, you will call it from your C/C++ code later
    @TargetApi(19)
    fun setImmersiveSticky() {
        val decorView = window.decorView
        decorView.systemUiVisibility = (View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_LAYOUT_STABLE)
    }

    var _activity: vulkanNativeActivity? = null
    var _popupWindow: PopupWindow? = null
    var _label: TextView? = null
    @SuppressLint("InflateParams")
    fun showUI() {
        if (_popupWindow != null) return
        _activity = this
        runOnUiThread {
            val layoutInflater = baseContext
                .getSystemService(LAYOUT_INFLATER_SERVICE) as LayoutInflater
            val popupView = layoutInflater.inflate(R.layout.widgets, null)
            _popupWindow = PopupWindow(
                popupView,
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT
            )
            val mainLayout = LinearLayout(_activity)
            val params = MarginLayoutParams(
                WindowManager.LayoutParams.WRAP_CONTENT,
                WindowManager.LayoutParams.WRAP_CONTENT
            )
            params.setMargins(0, 0, 0, 0)
            _activity!!.setContentView(mainLayout, params)

            // Show our UI over NativeActivity window
            _popupWindow!!.showAtLocation(mainLayout, Gravity.TOP or Gravity.START, 10, 10)
            _popupWindow!!.update()
            _label = popupView.findViewById<View>(R.id.textViewFPS) as TextView
        }
    }

    override fun onPause() {
        super.onPause()
    }

    fun updateFPS(fFPS: Float) {
        if (_label == null) return
        _activity = this
        runOnUiThread {
            _label!!.text = String.format("%2.2f FPS", fFPS)
            //_label.setText(String.format("test", "test"));
        }
    }

    fun openDirectory(pickerInitialUri: Uri) {
        // Choose a directory using the system's file picker.
        val intent = Intent(Intent.ACTION_OPEN_DOCUMENT_TREE).apply {
            // Optionally, specify a URI for the directory that should be opened in
            // the system file picker when it loads.
            putExtra(DocumentsContract.EXTRA_INITIAL_URI, pickerInitialUri)
        }

        startActivityForResult(intent, 0)
    }

    companion object{
        init {
            System.loadLibrary("vulkanNativeActivity")
        }
        @JvmStatic
        fun testFun(){
            Log.d("aqoole java","test function called")
        }
    }
}
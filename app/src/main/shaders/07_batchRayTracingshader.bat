@rem /usr/bin/glslangValidator --target-env spirv1.4 -l/07_raycommon.glsl -V 07_raygen.rgen -o 07_raygenRgen.spv
set TOOL_DIR=C:\Users\kodai\AppData\Local\Android\Sdk\ndk\25.0.8221429\shader-tools\windows-x86_64
@rem set DST_DIR=C:\Users\kodai\AndroidStudioProjects\vulkanTutorial\android-vulkan-tutorials_myRepo\Aqoole_project00\app\build\intermediates\assets\debug\mergeDebugAssets\shaders
set DST_DIR=C:\Users\kodai\AndroidStudioProjects\vulkanTutorial\android-vulkan-tutorials_myRepo\Aqoole_project00\app\build\intermediates\assets\debug\shaders
setlocal
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_raygen.rgen -o %DST_DIR%\07_raygenRgen.spv
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_raymiss.rmiss -o %DST_DIR%\07_rayRmiss.spv
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_shadow.rmiss -o %DST_DIR%\07_shadowRmiss.spv
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_closestHit.rchit -o %DST_DIR%\07_rayRchit.spv
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_colorBlend.rchit -o %DST_DIR%\07_colorBlendRchit.spv
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_animationVertex.comp -o %DST_DIR%\07_animationComp.spv
%TOOL_DIR%\glslc --target-spv=spv1.4 -std=460 07_animationGltf.comp -o %DST_DIR%\07_animationGltfComp.spv
@rem #/usr/bin/glslc --target-spv=spv1.4 -std=460 07_rayQueue.frag -o 07_rayQueueFrag.spv
@rem #/usr/bin/glslc --target-spv=spv1.4 -std=460 07_causticsGen.rgen -o 07_causticsRgen.spv
@rem #/usr/bin/glslc --target-spv=spv1.4 -std=460 07_causticsChit.rchit -o 07_causticsRchit.spv
@rem #/usr/bin/glslc --target-spv=spv1.4 -std=460 07_closestHit2.rchit -o 07_rayRchit2.spv
endlocal
pause

#!/bin/sh
#/usr/bin/glslangValidator --target-env spirv1.4 -l/07_raycommon.glsl -V 07_raygen.rgen -o 07_raygenRgen.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_raygen.rgen -o 07_raygenRgen.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_raymiss.rmiss -o 07_rayRmiss.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_shadow.rmiss -o 07_shadowRmiss.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_closestHit.rchit -o 07_rayRchit.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_colorBlend.rchit -o 07_colorBlendRchit.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_waterCompute.comp -o 07_waterComputeComp.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_rayQueue.frag -o 07_rayQueueFrag.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_causticsGen.rgen -o 07_causticsRgen.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_causticsChit.rchit -o 07_causticsRchit.spv
/usr/bin/glslc --target-spv=spv1.4 -std=460 07_closestHit2.rchit -o 07_rayRchit2.spv


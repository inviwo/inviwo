/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 *********************************************************************************/

#ifndef RUNNING_MEAN_AND_STANDARD_DEVIATION_CL
#define RUNNING_MEAN_AND_STANDARD_DEVIATION_CL

#include "samplers.cl" 
/*
 * Compute running average and standard deviation given new samples
 * http://en.wikipedia.org/wiki/Standard_deviation (Rapid calculation methods)
 */

void runningMeanAndStandardDeviation(int iteration, float multiplier, float4 sample,
									 float4* __restrict mean, float4* __restrict std)
{
	const float4 oldM = *mean;
	const float4 oldS = *std;
	const float N = (float)iteration;

	if(iteration <= 1)
	{
		*mean = (float4)(sample.xyz,1.f);
		*std = (float4)(0.f,0.f,0.f,1.f);
	}
	else
	{
		float4 newM = oldM + (sample - oldM)/N;
		newM.w = 1.f;
		float4 Qprev = (N-2.f)*oldS/multiplier;
		float4 Q = Qprev + (sample-oldM)*(sample-newM);
		float4 newS = multiplier*Q/(N-1.f);
		newS.w = 1.f;
		
		*mean = newM;
		*std = newS;
	}
}

#endif
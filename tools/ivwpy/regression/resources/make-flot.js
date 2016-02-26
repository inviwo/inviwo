/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

var options = {
    yaxes: [ { 
        show : true,
        position : "right",
    }, {
        show : true,
        position : "left",
        min : 0,
    }],
    xaxis : {
        mode: "time",
        timeformat: "%e %b",
    },
    grid: {
        show: true,
        borderColor: "gray"
    },
    legend: {
        show : true,
        position : "sw",
        backgroundColor : null,
        backgroundOpacity : 0,
        noColumns : 3
    }
};

$.plot($("#flot-summary"), 
    [{
        lines: { 
            show : true, 
            fill : true,
            steps : true,
            fillColor : "#E4FBE4;"
        },
        points: { show : false },
        label : "Successes",
        data  : passdata,
        color : "#82DB82",
        stack : "test",
        yaxis : 2,
        fill  : 0
    }
    ,
    {
        lines: { 
            show : true, 
            fill  : true,
            steps  : true,
            fillColor : "#FFE7E7"
        },
        points: { show: false },
        label : "Failues/Disabled",
        data  : faildata,
        color : "#FF9898",
        stack : "test",
        yaxis : 2,
        fill  : 1
    },
    {
        lines: { show: true },
        points: { show: true },
        label : "Normalized run time",
        data  : summarydata,
        color : "#907FC6",
        yaxis : 1
    }
    ],
    options 
);
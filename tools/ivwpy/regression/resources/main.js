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
 
$(document).ready(function() {
   	$('div.lihead').click(function() {
   			body = $(this).next(".libody") 			
   			body.slideToggle(100);
           	$.sparkline_display_visible()
    });

	$('.sparkline_elapsed_time').sparkline('html', {
		type : 'line', 
		width : '75px', 
		enableTagOptions: true,
		fillColor : false,
		lineColor : '#473386',
		spotColor : '#C3AC3B',
		minSpotColor : '#C3AC3B',
		maxSpotColor : '#C3AC3B',
		normalRangeColor: '#B3EEB3',
		drawNormalOnTop: false,
		tooltipFormatFieldlist : ['x', 'y'],
		tooltipFormatter :  function(sp, options, fields) {
			var retval = "";
			if (!fields.isNull) {
				var dateString = new Date(1000*fields.x).toLocaleString();
				retval += '<div class="jqsfield">';
				retval += 'Run time ' + dateString + ' : ' + fields.y.toPrecision(6) + 's';
			    retval += '</div>';
			}
			return retval;
		}
	});

	$('.sparkline_img_diff').sparkline('html', {
		type : 'line', 
		width : '75px', 
		enableTagOptions: true,
		fillColor : false,
		lineColor : '#473386',
		spotColor : '#C3AC3B',
		minSpotColor : '#C3AC3B',
		maxSpotColor : '#C3AC3B',
		normalRangeColor: '#B3EEB3',
		drawNormalOnTop: false,
		tooltipFormatFieldlist : ['x', 'y'],
		tooltipFormatter :  function(sp, options, fields) {
			var retval = "";
			if (!fields.isNull) {
				var dateString = new Date(1000*fields.x).toLocaleString();
				retval += '<div class="jqsfield">';
				retval += 'Diff ' + dateString + ' : ' + fields.y.toPrecision(8) + '%';
			    retval += '</div>';
			}
			return retval;
		}
	});
	
	$('.sparkline-failues').sparkline('html', {
		type : 'line', 
		width : '75px', 
		enableTagOptions: true,
		fillColor : false,
		lineColor : '#473386',
		spotColor : '#C3AC3B',
		minSpotColor : '#C3AC3B',
		maxSpotColor : '#C3AC3B',
		chartRangeMin : 0, 
		chartRangeMax : 6,
		normalRangeColor: '#B3EEB3',
		normalRangeMin : -0.5,
		normalRangeMax : 0.5,
		drawNormalOnTop: true,
		tooltipFormatter :  function(sp, options, fields) {
			var retval = "";
			if (!fields.isNull) {
				var dateString = new Date(1000*fields.x).toLocaleString();
				retval += '<div class="jqsfield">';
				retval += 'Failures ' + dateString + ' : ' + fields.y.toPrecision(8);
			    retval += '</div>';
			}
			return retval;
		}
	});

	$('.zoomset').zoom({magnify : 4, on : 'grab', duration : 400});
	
	userList.sort('testname');
	userList.sort('testfailures', { order: "desc" });
	userList.sort('testmodule');
 });
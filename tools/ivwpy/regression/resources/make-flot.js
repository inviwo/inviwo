/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2022 Inviwo Foundation
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

function average(data) {
    var sum = data.reduce(function(sum, value) {
        return sum + value;
    }, 0);

    var avg = sum / data.length;
    return avg;
}

function standardDeviation(values){
    var avg = average(values);

    var squareDiffs = values.map(function(value){
        var diff = value - avg;
        var sqrDiff = diff * diff;
        return sqrDiff;
    });
  
    var avgSquareDiff = average(squareDiffs);

    var stdDev = Math.sqrt(avgSquareDiff);
    return stdDev;
}

function stdRange(data) {
    values = data.map( function([x,y]){return y;} );
    var avg = average(values);
    var std = standardDeviation(values);
    var minval = Math.min(...values);
    var maxval = Math.max(...values);
    var ymin = Math.min(values[values.length - 1], Math.max(minval, avg - 3*std));
    var ymax = Math.max(values[values.length - 1], Math.min(maxval, avg + 3*std));

    return [ymin, ymax];
}

var summary_range = stdRange(summarydata);

var summary_options = {
    yaxes: [ { 
        show : true,
        position : "right",
        min : summary_range[0],
        max : summary_range[1],
        zoomRange: [0.01, 10],
        autoScale: "loose",
        autoScaleMargin: 0.02,
    }, {
        show : true,
        position : "left",
        min : 0,
    }],
    xaxis : {
        show: true,
        mode: "time",
        timeBase: "milliseconds",
        timeformat: "%e %b<br>%H:%M",
        zoomRange: [0.1, 100000],
        timezone: "browser"
    },
    grid: {
        show: true,
        borderColor: "gray",
        hoverable: true,
    },
    zoom: {
        interactive: false
    },
    selection: {
        mode: "xy"
    },
    legend: {
        show : true,
        position : "sw",
        noColumns : 4,
    }
};

summary_data = [
    {
        lines: { 
            show : true, 
            //fill : true, some fire fox problem...
            steps : false,
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
            steps  : false,
            fillColor : "#FFE7E7"
        },
        points: { show: false },
        label : "Failures",
        data  : faildata,
        color : "#FF9898",
        stack : "test",
        yaxis : 2,
        fill  : 1
    }
    ,
    {
        lines: { 
            show : true, 
            fill  : true,
            steps  : false,
            fillColor : "#FFFBE7"
        },
        points: { show: false },
        label : "Disabled",
        data  : skipdata,
        color : "#FFED98",
        stack : "test",
        yaxis : 2,
        fill  : 1
    }
    ,
    {
        lines: { show: true },
        points: { show: true },
        label : "Normalized run time",
        data  : summarydata,
        color : "#907FC6",
        yaxis : 1
    }
];

var plot = $.plot("#flot-summary", summary_data, summary_options);

$("#flot-summary").dblclick(function () {
    plot = $.plot("#flot-summary", summary_data, summary_options);
});

$("#flot-summary").bind("plotselected", function (event, ranges) {
    // clamp the zooming to prevent eternal zoom
    if (ranges.xaxis.to - ranges.xaxis.from < 0.00001) {
        ranges.xaxis.to = ranges.xaxis.from + 0.00001;
    }
    if (ranges.yaxis.to - ranges.yaxis.from < 0.00001) {
        ranges.yaxis.to = ranges.yaxis.from + 0.00001;
    }

    var axes = plot.getAxes();
    axes.xaxis.options.min = ranges.xaxis.from;
    axes.xaxis.options.max = ranges.xaxis.to;
    // zoom only first y axis
    axes.yaxis.options.min = ranges.yaxis.from;
    axes.yaxis.options.max = ranges.yaxis.to;
    
    plot.setupGrid();
    plot.draw();
    plot.clearSelection();
});

$("<div id='flotTooltip'></div>").appendTo("body");

$("#flot-summary").bind("plothover", function (event, pos, item) {
    if (!pos.x || !pos.y) {
        return;
    }

    if (item) {
        var date = new Date(item.datapoint[0]);
        var dateStr = $.plot.formatDate(date, "%Y-%m-%d %H:%M:%S");
        var value = item.datapoint[1].toString();

        $("#flotTooltip").html("<div><b>" + item.series.label + ":</b> " + value + "</div><div>" + dateStr + "</div")
            .css({top: item.pageY+8, left: item.pageX+8})
            .fadeIn(200);
    } else {
        $("#flotTooltip").stop().hide();
    }
});

$("#flot-summary").bind("plothovercleanup", function (event, pos, item) {
        $("#flotTooltip").hide();
});



function makePlot(elem, data) {
    var options = {
        xaxis : {
            show: true,
            mode: "time",
            timeBase: "milliseconds",
            timeformat: "%e %b<br>%H:%M",
            zoomRange: [0.1, 100000],
            timezone: "browser"
        },
        yaxis : {
            show: true,
            position : "left"
        },
        grid: {
            hoverable: true,
            borderColor: "gray",
        },
        tooltip: {
            show: true,
            xDateFormat: "%Y-%m-%d %H:%M:%S",
            content: "%s | %x, %y.8"
        },
        legend: {
            show : true,
            position : "ne",
            labelFormatter: function(label, series) {
                // series is the series object for the label
                return label.replaceAll('_', ' ');
            },
        },
        lines: { show: true },
        points: { show: true },
        selection: {
            mode: "xy"
        }
    };

    $(elem).plot(data, options);

    $(elem).dblclick(function () {
        $(elem).plot(data, options);        
    });
    
    $(elem).bind("plotselected", function (event, ranges) {
        // clamp the zooming to prevent eternal zoom
        if (ranges.xaxis.to - ranges.xaxis.from < 0.00001) {
            ranges.xaxis.to = ranges.xaxis.from + 0.00001;
        }

        if (ranges.yaxis.to - ranges.yaxis.from < 0.00001) {
            ranges.yaxis.to = ranges.yaxis.from + 0.00001;
        }

        let plot = $(elem).data("plot");
        var axes = plot.getAxes();
        axes.xaxis.options.min = ranges.xaxis.from;
        axes.xaxis.options.max = ranges.xaxis.to;
        // zoom only first y axis
        axes.yaxis.options.min = ranges.yaxis.from;
        axes.yaxis.options.max = ranges.yaxis.to;
        
        plot.setupGrid();
        plot.draw();
        plot.clearSelection();
    });

    $(elem).bind("plothover", function (event, pos, item) {
        if (!pos.x || !pos.y) {
            return;
        }

        if (item) {
            let date = new Date(item.datapoint[0]);
            let dateStr = $.plot.formatDate(date, "%Y-%m-%d %H:%M:%S");
            let value = item.datapoint[1].toString();

            let label = item.series.label.replaceAll('_', ' ');

            $("#flotTooltip").html("<div><b>" + label + ":</b> " + value + "</div><div>" + dateStr + "</div")
                .css({top: item.pageY+8, left: item.pageX+8})
                .fadeIn(200);
        } else {
            $("#flotTooltip").stop().hide();
        }
    });

    $(elem).bind("plothovercleanup", function (event, pos, item) {
            $("#flotTooltip").hide();
    });
}

var groupBy = function(xs, f) {
  return xs.reduce(function(rv, x) {
    (rv[f(x)] = rv[f(x)] || []).push(x);
    return rv;
  }, {});
};

var plotted = {}
$(".flot-plots").each(function (i, item) {
    var cont = $(item);
    var observer = new MutationObserver(function (mutations) {
        if (!(cont.attr('id') in plotted)) {
            plotted[cont.attr('id')] = true;
            var data = plotdata[cont.attr('id')];
            groups = groupBy(Object.keys(data), function (x) { return x.split(/\.|-/)[0];})

            Object.keys(groups).forEach(function (key) {
                var elem = $("<div class='flot-container'></div>)");
                cont.append(elem);

                datalist = groups[key].map(function (akey) {
                    return {
                        data : data[akey]["data"], 
                        label : akey + " (" + data[akey]["unit"] + ")"
                    }
                });
                makePlot(elem, datalist);
            });
        }   
    });
    observer.observe(cont.closest(".libody")[0], { attributes: true, attributeFilter: ['style']});
});
import * as d3 from "https://cdn.jsdelivr.net/npm/d3@7/+esm";

// Code from d3-graph-gallery.com

// create a pivot table with one entry per row from a DataFrame table
//
// Example DataFrame input:
//    [
//      "columns":  [ "col 1", "col 2", "col 3" ],
//      "data": [
//        [ 0, 1, 3 ],
//        [ 1, 2, 6 ]
//      ]
//    ]
// output:
//    [
//      { "col": "col 1", "row": 1, "value": 0 },
//      { "col": "col 2", "row": 1, "value": 1 },
//      { "col": "col 3", "row": 1, "value": 3 },
//      { "col": "col 1", "row": 2, "value": 1 },
//      ...
//    ]
export function createPivotTable(table) {
  const columnNames = table["columns"]

  return table["data"].reduce((acc, row, rowIndex) => {
    for (const [colIndex, col] of columnNames.entries()) {
      acc.push({
        "col": col, "row": rowIndex + 1, "value": row[colIndex],
        "colIndex": colIndex, "rowIndex": rowIndex
      });
    }
    return acc;
  }, []);
}


export function plot(holder, data, dataRange, size, margin, colorScale = d3.interpolateInferno, spacing = 0.1) {

  const width = size.width - margin.left - margin.right;
  const height = size.height - margin.top - margin.bottom;

  const svg = holder.append("svg")
      .attr("width", size.width)
      .attr("height", size.height);
  const plot = svg.append("g")
          .attr("id", "plottransform")
          .attr("transform", `translate(${margin.left}, ${margin.top})`)
        .append("g")
          .attr("id", "plot");

  // Labels of row and columns -> unique identifier of the column called 'col' and 'row'
  const myCols = Array.from(new Set(data.map(d => d.col)))
  const myRows = Array.from(new Set(data.map(d => d.row)))

  // Build X scales and axis:
  const x = d3.scaleBand()
    .range([ 0, width ])
    .domain(myCols)
    .padding(spacing);

  const xAxis = plot.append("g")
    .style("font-size", 15)
    .attr("transform", `translate(0, ${height})`)
    .call(d3.axisBottom(x).tickSize(0));
  xAxis.select(".domain").remove();

  xAxis.selectAll("text")
    .style("text-anchor", "end")
    .attr("dx", "-.4em")
    .attr("dy", ".5em")
    .attr("transform", "rotate(-45)");

  // Build Y scales and axis:
  const y = d3.scaleBand()
    .range([ height, 0 ])
    .domain(myRows)
    .padding(spacing);

  const yAxis = plot.append("g")
    .style("font-size", 15)
    .call(d3.axisLeft(y).tickSize(0));
  yAxis.select(".domain").remove();

  // Build color scale
  const color = d3.scaleSequential()
    .domain(dataRange)
    .interpolator(colorScale)


  // add the squares
  plot.selectAll("rect")
    .data(data, function(d) {return d.col+':'+d.row;})
    .join("rect")
      .attr("x", function(d) { return x(d.col) })
      .attr("y", function(d) { return y(d.row) })
      .attr("rx", 2)
      .attr("ry", 2)
      .attr("width", x.bandwidth() )
      .attr("height", y.bandwidth() )
      .style("fill", function(d) { return color(d.value)} )
      .style("stroke-width", 4)
      .style("stroke", "none");

    return {grid: plot, svg: svg, xAxis: xAxis, yAxis: yAxis}
}

export function rgba(color) {
    const r = Math.round(color[0]*255);
    const g = Math.round(color[1]*255);
    const b = Math.round(color[2]*255);
    const a = Math.round(color[3]*255);
    return `rgba(${r},${g},${b},${a})`
}

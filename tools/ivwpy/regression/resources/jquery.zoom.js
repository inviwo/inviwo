/*!
	Zoom 1.7.14
	license: MIT
	http://www.jacklmoore.com/zoom
*/
(function ($) {
	var defaults = {
		url: false,
		callback: false,
		target: false,
		duration: 120,
		on: 'mouseover', // other options: grab, click, toggle
		touch: true, // enables a touch fallback
		onZoomIn: false,
		onZoomOut: false,
		magnify: 1
	};

	// Core Zoom Logic, independent of event listeners.
	$.zoom = function(target, source, img, magnify) {
		var targetHeight,
			targetWidth,
			sourceHeight,
			sourceWidth,
			xRatio,
			yRatio,
			offset,
			$target = $(target),
			position = $target.css('position'),
			$source = $(source);

		// The parent element needs positioning so that the zoomed element can be correctly positioned within.
		$target.css('position', /(absolute|fixed)/.test(position) ? position : 'relative');
		$target.css('overflow', 'hidden');

		img.style.width = img.style.height = '';

		$(img)
			.addClass('zoomImg')
			.css({
				position: 'absolute',
				top: 0,
				left: 0,
				opacity: 0,
				width: img.width * magnify,
				height: img.height * magnify,
				maxWidth: 'none',
				maxHeight: 'none'
			})
			.appendTo(target);

		return {
			init: function(mref) {
				targetWidth = $target.outerWidth();
				targetHeight = $target.outerHeight();

				if (source === $target[0]) {
					sourceWidth = targetWidth;
					sourceHeight = targetHeight;
				} else {
					sourceWidth = $source.outerWidth();
					sourceHeight = $source.outerHeight();
				}

				xRatio = (img.width - targetWidth) / sourceWidth;
				yRatio = (img.height - targetHeight) / sourceHeight;

				offset = $(mref).offset();
			},
			move: function (e) {
				var left = (e.pageX - offset.left),
					top = (e.pageY - offset.top);

				top = Math.max(Math.min(top, sourceHeight), 0);
				left = Math.max(Math.min(left, sourceWidth), 0);

				img.style.left = (left * -xRatio) + 'px';
				img.style.top = (top * -yRatio) + 'px';
			}
		};
	};

	$.fn.zoom = function (options) {
		return this.each(function(i, zoomset) {
			imgset = $(zoomset).find('.zoom');
			zooms = [];
			imgset.each(function () {
				var
				settings = $.extend({}, defaults, options || {}),
				//target will display the zoomed image
				target = settings.target || this,
				//source will provide zoom location info (thumbnail)
				source = this,
	
				$source = $(source),
				$target = $(target),
				img = document.createElement('img'),
				$img = $(img),
				mousemove = 'mousemove.zoom',
				clicked = false,
				touched = false,
				$urlElement;

				// If a url wasn't specified, look for an image element.
				if (!settings.url) {
					$urlElement = $source.find('img');
					if ($urlElement[0]) {
						settings.url = $urlElement.data('src') || $urlElement.attr('src');
					}
					if (!settings.url) {
						return;
					}
				}
	
				(function(){
					var position = $target.css('position');
					var overflow = $target.css('overflow');
	
					$source.one('zoom.destroy', function(){
						$source.off(".zoom");
						$target.css('position', position);
						$target.css('overflow', overflow);
						$img.remove();
					});
					
				}());
	
				img.onload = function () {
					var zoom = $.zoom(target, source, img, settings.magnify, imgset);
					zooms.push(zoom);

					function start(e, mref) {
						Array.prototype.forEach.call(zooms, function(z){z.init(mref);});
						Array.prototype.forEach.call(zooms, function(z){z.move(e);});
						$(zoomset).find('.zoomImg').stop().fadeTo(settings.duration,1);
					}
	
					function stop() {
						$(zoomset).find('.zoomImg').stop().fadeTo(settings.duration,0);
					}

					function move(e) {
						Array.prototype.forEach.call(zooms, function(z){z.move(e);});
					}

	
					// Mouse events
					if (settings.on === 'grab') {
					$source
						.on('mousedown.zoom',
							function (e) {
								if (e.which === 1) {
									$(document).one('mouseup.zoom',
										function () {
											stop();
											$(document).off(mousemove, move);
										}
									);
									start(e, source);
									$(document).on(mousemove, move);
									e.preventDefault();
								}
							}
						);
					} else if (settings.on === 'click') {
					$source.on('click.zoom',
						function (e) {
							if (clicked) {
								// bubble the event up to the document to trigger the unbind.
								return;
							} else {
								clicked = true;
								start(e, source);
								$(document).on(mousemove, move);
								$(document).one('click.zoom',
									function () {
										stop();
										clicked = false;
										$(document).off(mousemove, move);
									}
								);
								return false;
							}
						}
					);
					} else if (settings.on === 'toggle') {
					$source.on('click.zoom',
						function (e) {
							if (clicked) {
								stop();
							} else {
								start(e, source);
							}
							clicked = !clicked;
						}
					);
					} else if (settings.on === 'mouseover') {
						zoom.init(); // Preemptively call init because IE7 will fire the mousemove handler before the hover handler.
	
						$source
							.on('mouseenter.zoom', function(e){start(e,source)})
							.on('mouseleave.zoom', stop)
							.on(mousemove, move);
					}
	
					// Touch fallback
					if (settings.touch) {
					$source
						.on('touchstart.zoom', function (e) {
							e.preventDefault();
							if (touched) {
								touched = false;
								stop();
							} else {
								touched = true;
								start( e.originalEvent.touches[0] || e.originalEvent.changedTouches[0], source);
							}
						})
						.on('touchmove.zoom', function (e) {
							e.preventDefault();
							move( e.originalEvent.touches[0] || e.originalEvent.changedTouches[0] );
						});
					}
					
					if ($.isFunction(settings.callback)) {
						settings.callback.call(img);
					}
				};
	
				img.src = settings.url;
			});
		});
	};

	$.fn.zoom.defaults = defaults;
}(window.jQuery));

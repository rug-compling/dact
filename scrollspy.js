(function($) {
	$.fn.scrollspy = function(content, offset)
	{
		var navigation = this,
			update_current_tab_timeout,
			links,
			offsets;

		function create_index()
		{
			links = $(navigation).find('a[href^=#]');

			offsets = $(content).find('h1, h2, h3').map(function() {
				return {
					'id': $(this).attr('id'),
					'top': $(this).position().top
				};
			});
		}

		function update_current_tab()
		{
			var reader_pos = window.scrollY,
				current;

			update_current_tab_timeout = 0;

			for (var i = 0; i < offsets.length; ++i)
			{
				current = offsets[i].id
					
				if (offsets[i+1] && offsets[i + 1].top > reader_pos)
					break;
			}

			links.each(function() {
				$(this).toggleClass('current', $(this).attr('href').substring(1) == current);
			});
		}

		// Just wait till the scrolling is over
		function taint_current_tab()
		{
			if (!update_current_tab_timeout)
				update_current_tab_timeout = setTimeout(update_current_tab, 75);
		}

		create_index();

		$(window).scroll(taint_current_tab);

		update_current_tab();
	}
})(jQuery);

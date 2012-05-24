(function() {
	function show_showcase(showcase) {
		showcase = $(showcase);
		$('#features-showcase').scrollTo(showcase, 500);

		$('#features-list a').each(function() {
			if (this.hash.substring(1) == showcase.attr('id'))
				$(this).addClass('active');
			else
				$(this).removeClass('active');
		});
	}
	
	$('#features-list a').click(function() {
		show_showcase(this.hash);
	});
	
	show_showcase($('#features-showcase > ul > li').first());

	var download_selector = document.getElementById('binaries');
	var binaries = download_selector.getElementsByTagName('div');

	$('#binaries > div').each(function() {
		if (navigator.platform.indexOf(this.className) != -1)
			$(this).show();
		else
			$(this).hide();
	});

	$('#show-all-downloads-button').click(function(e) {
		$('#binaries > div').show();
		this.style.visibility = 'hidden';
		e.preventDefault();
	});
	
	var scrollTo_settings = {
		offset: {left: 0, top: -30} // -30 compensates the menu that overlaps. h2's look nicer this way.
	};
	
	$('a[href^=#]')
		.filter(function() { return this.hash.length; })
		.click(function(e) {
			if ($(this.hash).parents('#features').length)
				$(window).scrollTo('#features', 500, scrollTo_settings);
			else
				$(window).scrollTo(this.hash, 500, scrollTo_settings);
			
			e.preventDefault();
		});
	
	// Don't recalculate the current tab while scrolling to prevent enormous lag.
	var update_current_tab_timeout;

	function update_current_tab()
	{
		$('article, header').each(function() {
			var size = $(this).height(),
				pos = $(this).position();

			// where is the current user with reading
			var reader_pos = window.scrollY + 160;

			$('a[href=#' + this.id + ']').toggleClass('current',
				reader_pos > pos.top && reader_pos < pos.top + size);
		});

		update_current_tab_timeout = null;
	}

	// Just wait till the scrolling is over
	function taint_current_tab()
	{
		if (!update_current_tab_timeout)
			update_current_tab_timeout = setTimeout(update_current_tab, 50);
	}

	$(window).scroll(taint_current_tab);
	update_current_tab();

	/*
	// Experimental full screen sections. That is a lot of empty space!
	function resize_sections() {
		$('header, article').css('min-height', $(window).height() + 'px');
	}
	
	$(window).bind('resize', resize_sections);
	resize_sections();
	*/
})();

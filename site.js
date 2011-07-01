(function() {
	function hide_showcases() {
		$('#features-showcase > div').hide();
	}
	
	function show_showcase(id) {
		$('#' + id).show();

		$('#features-list a').each(function() {
			if (this.hash.substring(1) == id)
				$(this).addClass('active');
			else
				$(this).removeClass('active');
		});
	}
	
	$('#features-list a').click(function() {
		hide_showcases();
		show_showcase(this.hash.substring(1));
	});
	
	hide_showcases();
	show_showcase($('#features-showcase > div')[0].id);

	var download_selector = document.getElementById('binaries');
	var binaries = download_selector.getElementsByTagName('div');

	$('#binaries > div').each(function() {
		if (this.className.indexOf(navigator.platform) != -1)
			$(this).show();
		else
			$(this).hide();
	});

	$('#show-all-downloads-button').click(function(e) {
		$('#binaries > div').show();
		this.style.visibility = 'hidden';
		e.preventDefault();
	});
	
	
	$('a[href^=#]')
		.filter(function() { return this.hash.length; })
		.click(function(e) {
			$(window).scrollTo(this.hash, 500);
			e.preventDefault();
		});
})();
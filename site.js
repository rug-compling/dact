(function() {
	var list = document.getElementById('features-list');
	var links = list.getElementsByTagName('a');
	var showcase_container = document.getElementById('features-showcase');
	var showcases = showcase_container.getElementsByTagName('div');

	function add_class(element, a_class) {
		element.className += ' ' + a_class;
	}

	function remove_class(element, a_class) {
		var pattern = new RegExp('\s*' + a_class + '\s*', 'g');
		element.className = element.className.replace(pattern, '');
	}
	
	function hide_showcases() {
		for (var i = 0; i < showcases.length; ++i)
			add_class(showcases[i], 'hidden');
	}
	
	function show_showcase(id) {
		var showcase = document.getElementById(id);
		remove_class(showcase, 'hidden');
	
		for (var i = 0; i < links.length; ++i)
			if (links[i].hash.substring(1) == id)
				add_class(links[i], 'active');
			else
				remove_class(links[i], 'active');
	}

	for (var i = 0; i < links.length; ++i) {
		links[i].onclick = function() {
			hide_showcases();
			show_showcase(this.hash.substring(1));
		}
	}
	
	hide_showcases();
	show_showcase(showcases[0].id);

	var download_selector = document.getElementById('binaries');
	var binaries = download_selector.getElementsByTagName('div');

	for (var i = 0; i < binaries.length; ++i) {
		if (binaries[i].className.indexOf(navigator.platform) != -1)
			remove_class(binaries[i], 'hidden');
		else
			add_class(binaries[i], 'hidden');
	}

	document.getElementById('show-all-downloads-button').onclick = function() {
		for (var i = 0; i < binaries.length; ++i)
			remove_class(binaries[i], 'hidden');
		
		this.style.visibility = 'hidden';
		return false;
	}
})();
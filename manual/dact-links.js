(function() {

function isMacro(code)
{
	return /^[a-z_][a-z0-9_]*\s+=\s+"""/.test(code);
}

function isQuery(code)
{
	return /^\s*\/\/[a-z_][a-z0-9_]*/.test(code);
}

function containsPlaceholders(code)
{
	return /%/.test(code);
}

function extractAllMacros(code, macros)
{
	var match, re = /([a-z_][a-z0-9_]*)\s*=\s*"""([\s\S]+?)"""/g;

	while (match = re.exec(code))
	{
		if (console.warn && macros[match[1]])
			console.warn('Redefining macro', match[1]);

		macros[match[1]] = match[2];
	}
}

function expandQuery(code, macros, stack)
{
	if (!containsPlaceholders(code))
		return code;

	return code.replace(
		/%([a-z_][a-z0-9_]*)%/gi,
		function(match, name) {
			return replaceInQuery(macros, name, stack);
		});
}

function condenseQuery(query)
{
	return query.replace(/\s+|\t/g, ' ');
}

function replaceInQuery(macros, name, stack)
{
	if (stack.indexOf(name) != -1)
		throw "Recursion detected!";

	if (!macros[name])
		throw "Unknown placeholder '" + name + "'";

	return expandQuery(macros[name], macros, stack.concat(name));
}

function linkQuery(query)
{
	query = condenseQuery(query);

	var link = document.createElement('a');
	link.href = 'dact:/?filter=' + encodeURIComponent(query);
	link.className = 'open-in-dact';
	link.appendChild(document.createTextNode('Run in Dact'));

	return link;
}

function annotateQuery(query, macros)
{
	var container = document.createElement('span'),
		chunks = query.split(/(%[a-z_][a-z0-9_]*%)/gi);

	container.className = 'annotated-query query';

	for (var i = 0; i < chunks.length; ++i)
	{
		var chunk = chunks[i];

		if (/^%.+%$/.test(chunk))
		{
			var placeholder = document.createElement('span');
			placeholder.className = 'placeholder'
			placeholder.title = replaceInQuery(macros, chunk.substr(1, chunk.length - 2), []).trim();
			placeholder.appendChild(document.createTextNode(chunk));
			container.appendChild(placeholder);
		}
		else
		{
			container.appendChild(document.createTextNode(chunk));
		}
	}

	return container;
}

function main()
{
	var blocksOfCode = document.getElementsByTagName('code'),
		macros = {};

	// First, find all the macros.
	for (var i = 0; i < blocksOfCode.length; ++i)
	{
		var code = blocksOfCode[i].textContent;

		try {
			if (isMacro(code))
				extractAllMacros(code, macros);
		} catch (e) {
			if (console.log)
				console.log(blocksOfCode[i], e);
		}
	}

	// Then, replace all the placeholders in the queries.
	for (var i = 0; i < blocksOfCode.length; ++i)
	{
		try {
			var code = blocksOfCode[i].textContent;

			if (isQuery(code))
			{
				blocksOfCode[i].className = 'query';

				// Add link to dact
				blocksOfCode[i].parentNode.appendChild(
					linkQuery(expandQuery(code, macros, [])));

				// Annotate placeholders
				// blocksOfCode[i].parentNode.replaceChild(
				// 	annotateQuery(code, macros), blocksOfCode[i]);
			}

			
			else if (isMacro(code))
			{
				// Annotate placeholders
				// blocksOfCode[i].parentNode.replaceChild(
				// 	annotateQuery(code, macros), blocksOfCode[i]);
			}
		} catch (e) {
			if (console.log)
				console.log(blocksOfCode[i], e);
		}
	}
}

main();

})();
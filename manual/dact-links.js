(function() {

function isMacro(code)
{
	return /^[a-z_]+\s+=\s+"""/.test(code);
}

function isQuery(code)
{
	return /^\/\/[a-z]+/.test(code);
}

function containsPlaceholders(code)
{
	return /%/.test(code);
}

function extractAllMacros(code, macros)
{
	var match, re = /([a-z_]+)\s*=\s*"""([\s\S]+?)"""/g;

	while (match = re.exec(code))
		macros[match[1]] = match[2];
}

function expandQuery(code, macros, stack)
{
	if (!containsPlaceholders(code))
		return code;

	return code.replace(
		/%([a-z_]+)%/gi,
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

function main()
{
	var blocksOfCode = document.getElementsByTagName('code'),
		macros = {};

	for (var i = 0; i < blocksOfCode.length; ++i)
	{
		var code = blocksOfCode[i].textContent;

		if (isMacro(code))
			extractAllMacros(code, macros);

		else if (isQuery(code))
		{
			blocksOfCode[i].className = 'query';
			
			blocksOfCode[i].parentNode.appendChild(
				linkQuery(expandQuery(code, macros, [])));
		}
	}
}

main();

})();
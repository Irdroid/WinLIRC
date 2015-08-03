function showPluginConf(pluginName, containerId)
{
	var elem = document.getElementById(containerId);
	elem.innerHTML = '<iframe src="config/' + pluginName + '.html" frameborder="0" seamless/>';
}
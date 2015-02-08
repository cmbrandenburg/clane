function ajax(uri, reqBody, callback) {
	if (window.XMLHttpRequest)
		var xhr = new XMLHttpRequest()
	else
		var xhr = new ActiveXObject('Microsoft.XMLHTTP')
	xhr.open('POST', uri)
	xhr.onreadystatechange = function() {
		if (xhr.readyState == 4 && xhr.status == 200) {
			callback(xhr.responseText)
		}
	}
	xhr.send(reqBody)
}

function count() {
	ajax('/count', '', function(respBody) {
		var x = JSON.parse(respBody)
		var n = parseInt(x.n)
		document.getElementById('count').innerHTML = x.n
		if (n == 1) {
			document.getElementById('other_browser').className = 'visible'
		} else if (n == 6) {
			document.getElementById('prisoner').className = 'visible'
		} else {
			document.getElementById('other_browser').className = 'hidden'
			document.getElementById('prisoner').className = 'hidden'
		}
	})
}


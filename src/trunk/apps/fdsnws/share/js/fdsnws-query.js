function serialize(form, loc='query') {
	if ( !form || form.nodeName !== "FORM" ) {
		return;
	}

	var query = loc;
	var element, option, i, j, q = [];
	for ( i = 0; element = form.elements[i]; i++ ) {
		if ( element.name === "" || element.disabled || element.offsetParent === null ) {
			continue;
		}

		// the queryauth input is treated separately
		if ( element.name === 'queryauth' ) {
			if ( element.checked ) {
				query = 'queryauth';
			}
			continue;
		}


		switch ( element.nodeName ) {
		case 'INPUT':
			if ( element.value == "" ) {
				break;
			}

			switch ( element.type ) {
			case 'text':
			case 'number':
			case 'hidden':
			case 'password':
			case 'button':
			case 'reset':
			case 'submit':
				q.push(element.name + "=" + encodeURIComponent(element.value));
				break;

			case 'checkbox':
			case 'radio':
				if ( element.checked ) {
					q.push(element.name + "=" + encodeURIComponent(element.value));
				}
				break;

			case 'file':
				break;
			}

			break;

		case 'TEXTAREA':
			if ( element.value == "" ) {
				break;
			}

			q.push(element.name + "=" + encodeURIComponent(element.value));
			break;

		case 'SELECT':
			switch ( element.type ) {
			case 'select-one':
				if ( element.value == "" ) {
					break;
				}

				q.push(element.name + "=" + encodeURIComponent(element.value));
				break;

			case 'select-multiple':
				values = null
				for ( j = 0; option = element.options[j]; j++ ) {
					if ( option.selected ) {
						v = encodeURIComponent(option.value)
						if ( values === null ) {
							values = v;
						}
						else {
							values += "," + v;
						}
					}
				}
				if ( values !== null ) {
					q.push(element.name + "=" + values);
				}
				break;
			}
			break;

		case 'BUTTON':
			if ( element.value == "" ) {
				break;
			}

			switch ( element.type ) {
			case 'reset':
			case 'submit':
			case 'button':
				q.push(element.name + "=" + encodeURIComponent(element.value));
				break;
			}
			break;
		}
	}

	var params = q.join("&");
	return params.length === 0 ? query : query + '?' + params;
}

function fdsnwsInitQueryForm() {
	return fdsnwsInitQueryFormLocation('query')
}

function fdsnwsInitQueryFormLocation(loc) {
	var queryForm = document.getElementById('query-form');
	var queryURL = document.getElementById('query-url');

	function updateQueryURL() {
		var path = window.location.pathname.split('/');
		path.pop();
		var url = window.location.origin + path.join('/') + '/' +
		          serialize(queryForm, loc)
		queryURL.setAttribute('href', url);
		queryURL.innerHTML = url;
	}

	function toggleLocation() {
		for ( i = 0; radio = locRadios[i]; i++ ) {
			var input = document.getElementById(radio.id + '-input');
			if ( input ) {
				//alert('input: ' + input.id);
				input.style.display = radio.checked ? 'block' : 'none';
			}
		}

		updateQueryURL();
	}

	var element, i;
	var elements = queryForm.getElementsByTagName('input');
	var locRadios = []
	for ( i = 0; element = elements[i]; i++ ) {
		if ( element.type === 'radio' && element.name === 'location' ) {
			locRadios.push(element);
			element.onclick = toggleLocation;
		}
		else {
			element.oninput = updateQueryURL
			element.onchange = updateQueryURL
		}
	}

	var elements = queryForm.getElementsByTagName('select');
	for ( i = 0; element = elements[i]; i++ ) {
		element.onchange = updateQueryURL
	}

	toggleLocation();
	updateQueryURL();
};

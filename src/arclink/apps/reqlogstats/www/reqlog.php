<?php

# Where the SQLite DB files live. This should
# be somewhere inaccessible from the web server.
$reqlogstats_db_dir="/home/sysop/reqlogstats/var";

################ Common methods [based on liveseis_config.php]
function offset($date, $offset) {
	# $date - a 10-char yyyy-mm-dd string
	# $offset - in days
	$now = date("Ymd");

	## There is no diff/add/sub on php 5.2
	$month = substr($date,5,2);
	$day = substr($date,8,2);
	$year = substr($date,0,4);
	$offset_sec = $offset*60*60*24;
	$n = date("Ymd", mktime(00,00,00,$month,$day,$year) + $offset_sec);

	return date("Y-m-d", mktime(00,00,00,$month,$day,$year) + $offset_sec);
}

function clean_date($text) {
  // ...must contain only numbers [0-9] and '-':
  // ...must be yyyy-mm-dd
  $text = strpbrk($text, '0123456789-');  // Ignore all before first digit.
  list($y, $m, $d) = sscanf($text, "%d-%d-%d"); // Ignores all after what matches.
  return sprintf("%04d-%02d-%02d", $y, $m, $d);
}

function month_prev($y, $m, $d) {
  $m = $m - 1;
  if ($m == 0) {
    $m = "12";
    $y = $y - 1;
  }
  return date("Y-m-d", mktime(0, 0, 0, $m, $d, $y));
}

function month_next($y, $m, $d) {
  $m = $m + 1;
  if ($m == 13) {
    $m = "01";
    $y = $y + 1;
  }
  return date("Y-m-d", mktime(0, 0, 0, $m, $d, $y));
}

######### Manipulate the SQLite db

class ReqLogSQ3 {
	private $db = "";
	private $filemtime = "";

	function __construct($file) {
		try {
			if (is_file($file)) {
				$this->db = new PDO('sqlite:'.$file); 
				$this->filemtime = filemtime($file);
			} else {
				echo('Database not found or is corrupted.');
				die();
			}
		} catch (Exception $e) {
			die($e->GetMessage());
		}
	}

       private function result2table($result, $key = "") {
	 //echo "Fetch[$key]:".$result->rowCount();
	  $rows=array();
	  if (empty($result)) return $rows;
	  if (!empty($key))
	    while ($r = $result->fetch()) {
	     $k = $r[$key];
	     unset($r[$key]);
	     $rows[$k]=$r;
	    }
	  else
	    while ($r = $result->fetch()) $rows[] = $r;
	  return $rows;
	}

       function makequerytable($sql, $tname, $asTable = True) {
	 $q = str_replace('{table}', $tname, $sql);
	 $re = $this->db->query($q);
	 if (!$re) {
	   $error = $this->db->errorInfo();
	   echo "Command: " . $q . PHP_EOL;
	   echo "Error: " . $error[1] . ":" . $error[2] . PHP_EOL;
	 }
	 if (!$asTable) return $re;

	 //	 foreach ($re as $row) {
	 //  var_dump($row);
	 //}


	 return $this->result2table($re);
       }

	function makepreparetable($sql, $tname, $vars, $asTable = True) {
	  $q = str_replace('{table}', $tname, $sql);
	  $t = $this->db->prepare($q);
	  $re = $t->execute($vars);
	  if (!$re) {
	    $error = $this->db->errorInfo();
	    echo "Command: " . $q . PHP_EOL;
	    echo "Vars: " . print_r($vars) . PHP_EOL;
	    echo "Error: " . $error[1] . ":" . $error[2] . PHP_EOL;
	  }

	  //echo "Response:";
	  //var_dump($re);

	  if (!$asTable) return $re;

	  if (($re !== True) and ($re !== False)) {
	    return $re->fetchAll();
	  } else {
	    return $re;
	  }

	  return $this->result2table($re);
	}

	function timeStamp() {
		// Store mtime at time the db connection is opened.
		// Otherwise someone can update the file, but this
		// db connection sees an older version?
		return date("Y M d H:i:s", $this->filemtime);
	}
}


// ----------------------------------------------------------------------

function tag($tag, $s, $attrs = "") {
  $attributes = "";
  if (is_array($attrs)) {
      foreach ($attrs as $k => $v) {
	$attributes = $attributes . ' ' . $k .'="' . $v .'"';
      }
    }
  return "<$tag$attributes>$s</$tag>";
}

function tag_list($tag, $a, $attrs_all = "", $attrs_some = Array()) {
    // All items of $a get marked up with tag $tag, so:
    //   "<tag>a1</tag><tag>a2</tag><tag>a3</tag>..."
    // Optional:
    //   $attrs_all: all <tag> elements will have these attrs.
    //   $attrs_some: a list - if element j is (k, v) then
    //      the jth item in $a gets a tag with attribute k='v'.
    //      NOTE: $attrs_some is searched for all len($a) elements,
    //      so use it sparingly.
    $s = "";
    $index = 0;
    foreach ($a as $item) {
        $index += 1;
        if (array_key_exists($index, $attrs_some)) {
	    $attrs = $attrs_all;
	    list($k, $v) = explode("=", $attrs_some[$index], 2);
	    $attrs[$k] = trim($v, '"');
	} else {
	    $attrs = $attrs_all;
	}
	$s = $s . tag($tag, $item, $attrs);
    }
    unset($item);
    return $s;
}

function h2($s) {
  return "<h2>$s</h2>";
}

function tr($s) {
  return tag("tr", $s);
}

function td($s) {
  return tag("td", $s);
}

function bytes_rounded($val) {
    // Return $val in units of kiB / MiB / GiB as appropriate,
    // as a *string*, with at most 3 decimal places.
    $byte_units = Array("kiB" => 1024, "MiB" => 1024*1024, "GiB" => 1024*1024*1024);
    foreach (Array("GiB", "MiB", "kiB") as $size) {
	if ($val > $byte_units[$size]) {
	    return rtrim(sprintf("%.3f", $val/$byte_units[$size]), "0") . " $size";
	}
    }
    return $val;
}

function sum_cols($data, $cols) {
    //  $data     - an Array of Arrays - one for header, one for data rows.
    //  $cols     - an Array of those column headers which are to
    //              be summed.

    $result = Array();
    foreach ($cols as $c => $v) {
	$result[$c] = 0;
    }

    //$headers = $data[0]; // First object
    $flipped_headers = array_flip($data[0]);
    $index = Array();
    $rounded = Array();
    foreach ($cols as $c => $v) {
	$index[$c] = $flipped_headers[$c];
	$rounded[$c] = False;
    }

    $byte_units = Array("kiB" => 1024, "MiB" => 1024*1024, "GiB" => 1024*1024*1024);

    foreach ($data[1] as $row) {
	foreach ($cols as $c => $v) {
	    if (array_key_exists($index[$c], $row)) {
		$val = $row[$index[$c]];
		if (substr($val, -2) == "iB") {
			$rounded[$c] = True;
			$unit = substr($val, -3);
			if (array_key_exists($unit, $byte_units)) {
				$val = floatval(substr($val, 0, -3))*$byte_units[$unit];
			} else {
				$val = 0;
			}
		}
		$result[$c] += $val;
	    }
	}
    }

    foreach ($cols as $c => $v) {
        if ($rounded[$c]) {
	    $result[$c] = bytes_rounded($result[$c]);
	}
    }
    return $result;
}


function render_table($format, $sources, $data, $options) {
    // Parameters:
    //  $format   - string, e.g. "html"
    //  $data     - an Array of Arrays - one for header, one for data rows.
    //  $options  - summable - an Array of column header names which
    //              are to be summed (across all rows i.e. vertically).
    //              linkcodes - boolean

    if (! isset($data) ) return;

    if (count($data) > 0) {
	$headers = $data[0]; // First object
	echo '<table width="100%">';
	echo '  ' . tag('thead', tr(tag_list("th", $headers))) . PHP_EOL;
	echo PHP_EOL;
	echo '  <tbody>' . PHP_EOL;

	$rows = $data[1]; // Second object
	//for($i = 1, $size = count($data); $i < $size; ++$i) {
	//  $row = $data[$i];
	foreach ($rows as $row) {
	  $selected = Array();
	  foreach ($headers as $col) {
	    if (array_key_exists($col, $row)) {
	      if ($col == 'source') {
		$selected[$col] = $sources['labels'][$row[$col]];
	      } else {
		$selected[$col] = $row[$col];
	      }
	    } else {
	      $selected[$col] = "??";
		// HACK as schema changed "source" to "src"
	      if ($col == 'source') {
		$selected[$col] = $sources['labels'][$row['src']];
	      }
	    }
	  };

          if (array_key_exists("networkCode", $row) && array_key_exists("linkcodes", $options) && $options["linkcodes"]) {
            $code = $selected["networkCode"];
            $hcode = str_replace("/", "_", $code);
            $selected["networkCode"] = tag("a", $code, Array('href' => "reqlognetwork.php?code=$hcode"));
          }

	  echo '  ' . tr(tag_list("td", $selected)) . PHP_EOL;
	}
	if (array_key_exists("summable", $options)) {
	  $summable_cols = $options["summable"];
	  // var_dump($summable_cols);

	  $col_sums = Array();
	  $col_totals = sum_cols($data, $summable_cols);
	  foreach ($headers as $col) {
	    if (array_key_exists($col, $summable_cols)) {
	      $col_total = $col_totals[$col];
	      $col_sums[] = tag('span', $col_total, Array("class" => "col_total"));
	    } else {
		$col_sums[] = "&nbsp;";
	    };
	  }
	  $col_sums[0] = "Column sums";
	  echo '  ' . tr(tag_list("td", $col_sums)) . PHP_EOL;
	}
        echo '  <tr><td>Rows: ' . count($rows) . '</td></tr>' . PHP_EOL;
	echo '  </tbody>' . PHP_EOL;
	echo '</table>';
    } else {
	var_dump($data);
    };
 
}

/* end of reqlog.php */

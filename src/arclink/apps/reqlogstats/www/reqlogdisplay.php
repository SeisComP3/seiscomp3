<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en" lang="en">
 <?php
 require "../../../geofon/GEconfig.php";
 loadCMS();
?>
<?php GEhead("Arclink Request Statistics", array("css=extra.css", "css=/eida/local_basic.css")); ?>

<body>
 <?php GEbar("Arclink Request Statistics"); ?>
 <?php GEmenu(); ?>
 <?php GEnavigation(); ?>
 <?php GEtop(); ?>

<?php

require 'reqlog.php';

// -----------------------------------------------------------------------------

function clean_dcid_list($text) {
  // Return array of well-behaved dcid names
  $pieces = explode(",", $text);
  foreach ($pieces as $word) {
    $text = trim(strtoupper($word));
    $text = strpbrk($text, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789');
    $dcid_list[] = $text;
  }
  return $dcid_list;
}

function mark_yesno($val) {
  if ($val != 0) {
    return tag('span', 'Y', Array('style' => 'color: green'));
  } else {
    return tag('span', 'N', Array('style' => 'color: red'));
  }
}

function render_availability_table($db, $start_str, $today_str, $end_str) {
  $result = $db->makequerytable("SELECT id, host, port, dcid FROM `{table}`", "ArcStatsSource", True);
  $num_sources = count($result);
  $source_list = array();
  foreach ($result as $row) {
    $source_list[] = $row['id'];
  }
  //var_dump($source_list);


  // Workaround for prepare/execute query problem with wildcarded ?, ?
  // which gave "Column Index out of range".
  $q = "SELECT count(*) FROM {table} WHERE `start_day` = ':start' AND `src` = ':source'";
  //TODO $thing = $db->preparequery($q);

  // Headers for the table:
  $day_list = Array();
  //$day = DateTime::createFromFormat('Y-m-d', $start_str); // silly!
  $day = $start_str; 
  $tmp = explode("-", $day);
  $tmp = month_next($tmp[0], $tmp[1], $tmp[2]);
  $finish = offset($tmp, -1);

  $me="reqlogdisplay.php";
  for (; $day <= $finish; $day = offset($day, 1)) {
    $day_list[] = tag('a', substr($day,8,3), Array('href' => "$me?date=$day"));
  }

  echo tag("p", "Reports available from $num_sources sources between $start_str and $end_str:");

  echo "<table>";
  echo tag('thead', tr(td('EIDA NODE') . tag_list('th', $day_list)));

  //foreach ($source_list as $source) {
  foreach ($result as $row) {
    $source_id = $row['id'];
    //$source_id = $source['id'];
    $dcid = $row['dcid'];
    $host = $row['host'];
    $port = $row['port'];

    $day = $start_str;  // DateTime::createFromFormat('Y-m-d', $start_str);
    $presence_list = Array();
    for (; $day <= $finish; $day = offset($day, 1)) {
      $day_str = $day;
      //TODO $result = queryexecute($thing, Array($day_str, $source_id));
      //$result = $db->makepreparetable($q, 'ArcStatsSummary', Array($day_str, $source_id));
      $q_2 = str_replace(Array(':start', ':source'),
			 Array($day_str, $source_id),
			 $q);
      //print "QUERY: " . $q_2 . PHP_EOL;

      $result = $db->makequerytable($q_2, 'ArcStatsSummary');
      if (is_array($result)) {
	$ans = $result[0];
	$yesno = (intval($ans[0]) > 0);
      } else {
	$yesno = False;
      }
      $presence_list[] = mark_yesno($yesno);
    }
    $header = tag("a", "$dcid", Array("href" => "$me?date=$today_str&dcid=$dcid")); // "$dcid ($host:$port)";
    $tmp = explode("-", $today_str);
    $highlight = Array(intval($tmp[2]) => 'style="border-style: Solid; border-color: yellow; border-width: 0px 1px; background-color: lightyellow;"');
    echo tr(td($header) . tag_list('td', $presence_list, "", $highlight)) . PHP_EOL;
  }

  echo "</table>";
}

// -----------------------------------------------------------------------------
date_default_timezone_set('UTC'); 

# May want to hide tables which reveal information about users:
$show_all_tables = isset($_GET['showall']);

$date_default = offset(date('Y-m-d'), -2);  // Two days ago

$dcid_list = (isset($_GET['dcid']))?$_GET['dcid']:'all';
$dcid_list = clean_dcid_list($dcid_list);

$date_str = (isset($_GET['date']))?$_GET['date']:$date_default;
// "The GET variables are passed through urldecode()."
$date_str = clean_date($date_str);
//$date = DateTime::createFromFormat('Y-m-d', $date_str); // e.g. '2006-12-12'
// Check for failure with DateTime::getLastErrors(): TODO
$date = $date_str;
$start_day = $date;

$prev_str = offset($date, -1); 
$next_str = offset($date, 1);

$last_day = Array(1 => 31, 29, 31,  30, 31, 30,  31, 31, 30,  31, 30, 31);
$month = intval(substr($date, 5, 2));
$month_start = substr($date,0,7) . '-01';
$month_start_str = $month_start;
$month_end_str = substr($date,0,7) . '-' . $last_day[$month];   // 31
unset($month_start);

#echo "open connection to sqlite file: dba/__construct";
$year = substr($date,0,4);
$month = substr($date,5,2);
$day = substr($date,8,2);

$db = new ReqLogSQ3($reqlogstats_db_dir . "/reqlogstats-$year.db");

$img_base_url = "../data";
$img1 = "$img_base_url/total-$year-$month.svg";
$img2 = "$img_base_url/sources-$year-$month.svg";

echo tag('h1', 'Arclink Request Statistics for ' . date('F Y', mktime(0, 0, 0, $month, $day, $year)));

if (file_exists($img1)) {
        print tag("p", '<a href="' . $img1 .'"><img src="' . $img1 . '"alt="Monthly chart total bytes" width="480"></a>  <a href="' . $img2 . '">Total_size by DCID</a>...');
} else {
	print tag("p", '[No graph for this month is available]');
}

echo tag("h3", "Table for $year");
$year_img = "../data/total-$year.svg";
if (file_exists($year_img)) {
	print tag("p", '<a href="' . $year_img .'"><img src="' . $year_img . '"alt="Year to date chart total bytes" width="480"></a> <a href="' . str_replace("total", "sources", $year_img) . '">Year to date by DCID</a>...');
}
print tag("p", 'Summary table for <a href="../data/total-' . $year. '.txt">' . $year . '</a>');

echo tag('h3', 'Reports received from');
render_availability_table($db, $month_start_str, $start_day, $month_end_str);

// echo tag("pick node or nodes to combine");

// The following must match the schema in reqlogstats.sql...
$table_list = Array( 'Summary', 'Request', 'Volume', 'Network', 'Messages' );
if ($show_all_tables) {
	$table_list[] = 'User';
	$table_list[] = 'UserIP';
	$table_list[] = 'ClientIP';
}

$table_heads = Array( 'Summary'  => Array('start_day', 'source', 'requests', 'requests_with_errors', 'error_count', 'users', 'stations', 'total_lines', 'total_size'),
		      'User'     => Array('start_day', 'source', 'userID', 'requests', 'lines', 'errors', 'size'),
		      'Request'  => Array('start_day', 'source', 'type', 'requests', 'lines', 'errors', 'size'),
		      'Volume'   => Array('start_day', 'source', 'type', 'requests', 'lines', 'errors', 'size'),
		      'Station'  => Array('start_day', 'source', 'streamID_networkCode', 'streamID_stationCode', 'stationID_locationCode', 'stationID_channelCode',  'requests', 'lines', 'errors', 'size', 'time'),
		      'Network'  => Array('start_day', 'source', 'networkCode', 'requests', 'lines', 'nodata', 'errors', 'size'),
		      'Messages' => Array('start_day', 'source', 'message', 'count'),
		      'UserIP'   => Array('start_day', 'source', 'userIP', 'requests', 'lines', 'errors', 'size'),
		      'ClientIP' => Array('start_day', 'source', 'clientIP', 'requests', 'lines', 'errors', 'size'),
		      );

$prev_month_str = month_prev($year, $month, $day);
$next_month_str = month_next($year, $month, $day);

$items = Array( tag('a', "&lt;&lt; Prev month", Array('href' => 'reqlogdisplay.php?date=' . $prev_month_str)) . ' |',
                tag('a', "&lt;&lt; Prev day", Array('href' => 'reqlogdisplay.php?date=' . $prev_str)) . ' |',
		"Summary statistics for ". $start_day,
		'| ' . tag('a', "Next day &gt;&gt;", Array('href' => 'reqlogdisplay.php?date=' . $next_str)),
                '| ' . tag('a', "Next month &gt;&gt;", Array('href' => 'reqlogdisplay.php?date=' . $next_month_str))
);
echo tag('table', tr(tag_list('td', $items)));
 

$marked_table_list=Array();
foreach ($table_list as $item) {
  $marked_table_list[] = tag("a", $item, Array('href' => "#Table$item"));
}
$table_nav = tag("ol", tag_list("li", $marked_table_list), Array('class' => 'nav'));
print tag("p", "Tables:") . $table_nav;

// Source data is needed for all tables:

$q =  "SELECT id, host, port, dcid FROM `{table}` ORDER BY `id`";
$result = $db->makequerytable($q, 'ArcStatsSource');
$labels = Array(0 => 'ALL');
foreach ($result as $row) {
  $labels[intval($row["id"])] = $row["dcid"];
}
$source_data = Array('labels' => $labels);

// Node IDs come from:
//  SELECT * from ArcStatsSource where host= and port=
$source_ids = array_flip($labels);
$selected_node_ids = Array();
foreach ($dcid_list as $dcid) {
  $selected_node_ids[] = $source_ids[$dcid];
}

print "Nodes included in this report: ";
foreach ($selected_node_ids as $node_id) {
  print " " . $labels[$node_id];
}

foreach ($table_list as $table) {
  echo h2("Table $table" . tag("a", "", Array('name' => "Table$table")));

    $options = Array();
    if (($table == "UserIP") || ($table == "ClientIP")) {
	$options["summable"] = array_flip(Array("requests", "lines", "errors"));
    } elseif ($table == "Summary") {
	$options["summable"] = array_flip(Array("requests", "requests_with_errors", "error_count", "total_lines", "total_size"));
    } elseif (($table == "User") || ($table == "Volume") || ($table == "Request")) {
	$options["summable"] = array_flip(Array("requests", "lines", "errors", "size"));
    } elseif ($table == "Network") {
	$options["summable"] = array_flip(Array("requests", "lines", "nodata", "errors", "size"));
	$options["linkcodes"] = True;
    };

    $order_col = $table_heads[$table][2];

    foreach ($selected_node_ids as $node_id) {
      if ($node_id == 0) {
        $source_constr = "";
      } else {
	$source_constr = "AND `src` = $node_id";
      }
      $order_col = $table_heads[$table][2];
      $q = "SELECT * FROM `{table}` WHERE `start_day` = '?' $source_constr ORDER BY `$order_col`";
      $tname = 'ArcStats' . $table;
      //$v = Array( $start_day );
      //$result = $db->makepreparetable($q, $tname, $v);

      $q_2 = str_replace('?', $start_day, $q);
      $result = $db->makequerytable($q_2, $tname);
      // echo "Got " . count($result) . " row(s)." ;
      $table_data = Array($table_heads[$table], $result);
      //var_dump($result);
      // TODO ... and combine this node's values
      echo PHP_EOL;
      render_table('html', $source_data, $table_data, $options);
    };
};

// One extra table:

$table = "Messages this month";
echo tag("h3", "Table $table" . tag("a", "", Array('name' => "TableMessages_this_month")));

$order_col = "message" ; // $table_heads[$table][2];
foreach ($selected_node_ids as $node_id) {
  if ($node_id == 0) {
    $source_constr = "";
  } else {
    $source_constr = "AND `src` = $node_id";
  }
  $date_constr = "WHERE start_day >= '$month_start_str' AND start_day <= '$month_end_str'";
  $q = "SELECT * FROM `{table}` " . $date_constr . " " . $source_constr . " ORDER BY `start_day`, `message`";
  $tname = 'ArcStatsMessages';

  $result = $db->makequerytable($q, $tname);
  // echo "Got " . count($result) . " row(s)." ;
  $table_data = Array($table_heads['Messages'], $result);
  //var_dump($result);
  echo PHP_EOL;
};

render_table('html', $source_data, $table_data, $options);

//var_dump($source_data);
//echo "close connection";

?>
 <?php GEbottom(); ?>
 <?php GEfooter(); ?>
</body>
</html>

<?php

function marketplace_get_predefined_param($param, $pre_array = array(), $suf_array = array()) {
  global $theme_key;
  $theme_data = list_themes();
  $result = isset($theme_data[$theme_key]->info[$param]) ? $theme_data[$theme_key]->info[$param] : array();
  return $pre_array + $result + $suf_array;
}

function marketplace_special_regions() {
  $special_regions = array('sidebar' => array(), 'panel' => array());
  global $theme_key;
  $theme_data = list_themes();
  $regions = $theme_data[$theme_key]->info['regions'];
  foreach ($regions as $key => $value) {
    $parts = explode("_", $key);
    foreach ($special_regions as $type => $info) {
      if (strpos($key, "sidebar_") === 0) {
        $special_regions['sidebar'][$key] = $key;
      }
      elseif ((strpos($key, "panel_") === 0) && count($parts) > 2) {
        $special_regions['panel'][$key] = $key;
      }
    }
  }
  return $special_regions;
}

function marketplace_panel_regions() {
  $special_regions = marketplace_special_regions();
  $panels = $special_regions['panel'];
  $panel_regions = array();
  foreach($panels as $region) {
    $parts = explode("_", $region);
    $panel = $parts[0] . "_" . $parts[1];
    if(!isset($panel_regions[$panel])) {
      $panel_regions[$panel] = array();
    }
    $panel_regions[$panel][] = $region;
  }
  return $panel_regions;
}

function marketplace_regions_width($page) {
    $special_regions = marketplace_special_regions();
    $regions_width = array();
    $main_region_width = 12;
    foreach($special_regions['sidebar'] as $region) {
      if ($markup = render($page[$region])) {
        $regions_width[$region] = theme_get_setting($region . "_width");
        $main_region_width -= $regions_width[$region];
      }
      else {
        $regions_width[$region] = 0;
      }
    }
    $regions_width['content'] = $main_region_width;
    $main_highlighted_width = 12;
    foreach($special_regions['panel'] as $region) {
      $regions_width[$region] = theme_get_setting($region . "_width");
      if (in_array($region, array('panel_highlighted_1', 'panel_highlighted_3')) ) {
        if ($markup = render($page[$region])) {
          $main_highlighted_width -= $regions_width[$region];
        }
        else {
          $regions_width[$region] = 0;
        }
      }
    }
    $regions_width['panel_highlighted_2'] = $main_highlighted_width;
    return $regions_width;
}

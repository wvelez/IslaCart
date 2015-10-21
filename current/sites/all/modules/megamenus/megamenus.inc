<?php

function megamenus_add_css($file) {
  $path_info = pathinfo($file);
  drupal_add_css($file, array(
    'group' => CSS_DEFAULT,
    'basename' => 'megamenus-' . $path_info['basename'],
  ));
}
/*
function megamenus_get_presets() {
  return array(
    'style_blue' => 'Blue',
    'style_classic' => 'Classic',
    'style_red' => 'Red',
    'theme_weebpal_education' => 'WeebPal Education',
    'theme_weebpal_corporate_site' => 'WeebPal Corporate Site',
    'theme_weebpal_hotel' => 'WeebPal Hotel',
    'theme_weebpal_ishopping' => 'WeebPal iShopping',
    'theme_weebpal_zircon' => 'WeebPal Zircon',
  	'theme_business' => 'Business',
  	'theme_flatize' => 'Flatize',
  	'theme_new_shopping' => 'New Shopping',
  );
}*/

function megamenus_get_blocks_info() {
  static $_blocks_array = array();
  if (empty($_blocks_array)) {
    $blocks = db_select('block', 'b')->fields('b')->execute()->fetchAll();
    $_blocks_array = array();
    foreach ($blocks as $block) {
      if ($block->module != 'megamenus') {
        $_blocks_array[$block->module . '--' . $block->delta] = $block;
      }
    }
  }
  return $_blocks_array;
}

function megamenus_get_blocks_options() {
  static $_blocks_array = array();
  if (empty($_blocks_array)) {
    $blocks = _block_rehash($GLOBALS['conf']['theme_default']);
    $_blocks_array = array();
    foreach ($blocks as $block) {
      if ($block['module'] != 'megamenus') {
        $_blocks_array[$block ['module'] . '--' . $block ['delta']] = empty($block['info']) ? $block['title'] : $block['info'];
      }
    }
    asort($_blocks_array);
  }
  return $_blocks_array;
}

function megamenus_get_block_title($block_key) {
  $blocks_options = megamenus_get_blocks_options();
  if(isset($blocks_options[$block_key])) {
    return $blocks_options[$block_key];
  }
  return NULL;
}

function megamenus_build_page_trail($page_menu) {
  $path = current_path();
  $trail = array ();
  foreach ($page_menu as $item) {
    if ($item ['link'] ['in_active_trail']) {
      $trail [$item ['link'] ['mlid']] = $item;
    }
    elseif ($item['link']['href'] == '<front>' && drupal_is_front_page()) {
      $trail [$item ['link'] ['mlid']] = $item;
    }
    if ($item ['below']) {
      $trail += megamenus_build_page_trail($item ['below']);
    }
  }
  return $trail;
}

function megamenus_get_megamenus() {
  $query = db_select('menu_custom', 'm');
  $query->leftJoin('megamenus', 't', 't.menu_name = m.menu_name');
  $query->fields('m');
  $menus = $query->execute()->fetchAll();
  return $menus;
}

function megamenus_get_menu($menu_name) {
  $query = db_select('menu_custom', 'm');
  $query->leftJoin('megamenus', 't', 't.menu_name = m.menu_name');
  $query->fields('m');
  $query->addField('t', 'menu_config');
  $query->addField('t', 'block_config');
  $query->condition('m.menu_name', $menu_name);
  return $query->execute()->fetchObject();
}

function megamenus_get_menu_config($menu_name, $items) {
  $menu = megamenus_get_menu($menu_name);
  $megamenu = array();
  if (!$menu || empty($menu->menu_config)) {
    $new_megamenu = array();

    $items = megamenus_get_tree($menu_name);

    foreach ($items as $key => $value) {
      if (!$value['link']['hidden']) {
        $new_megamenu[] = megamenus_load_item($value, 1);
      }
    }
    db_insert('megamenus')
      ->fields(array(
        'menu_name' => $menu_name,
        'menu_config' => json_encode($new_megamenu),
      ))
      ->execute();
    $megamenu = $new_megamenu;
  }
  else {
    $megamenu = json_decode($menu->menu_config);    
  }

  if(megamenus_is_different($megamenu, $items)) {
    $new_megamenu = array();
    foreach ($items as $key => $item) {
      if (!$item['link']['hidden']) {
        $megamenu_item = megamenus_find_submenu($item['link']['mlid'], $megamenu);
        
        if(megamenus_is_different_submenus($megamenu_item, $item, TRUE)) {
          $new_megamenu[] = megamenus_load_item($item, 1);
        }
        else {
          $new_megamenu[] = $megamenu_item;
        }
      }
    }

    db_update('megamenus')
      ->fields(array(
        'menu_config' => json_encode($new_megamenu),
      ))
      ->condition('menu_name', $menu_name)
      ->execute();
    $megamenu = $new_megamenu;
  }

  return $megamenu;  
}

function megamenus_get_block_config($menu_name) {
  $menu = megamenus_get_menu($menu_name);
  return $menu && isset($menu->block_config) ? json_decode($menu->block_config) : new stdClass();
}

function megamenus_find_templates($path) {
  $templates = array();
  $regex = '/\.tpl\.php$/';

  $files = drupal_system_listing($regex, $path, 'name', 0);
  foreach ($files as $template => $file) {
    if (($pos = strpos($template, '.')) !== FALSE) {
      $template = substr($template, 0, $pos);
    }
    $templates[$template] = $template;
  }

  return $templates;
}

function megamenus_get_tree($menu_name) {
  static $trees = array();
  if(!isset($trees[$menu_name])) {
    global $menu_admin;
    $sql = "
      SELECT m.load_functions, m.to_arg_functions, m.access_callback, m.access_arguments, m.page_callback, m.page_arguments, m.delivery_callback, m.title, m.title_callback, m.title_arguments, m.type, m.description, ml.*
      FROM {menu_links} ml LEFT JOIN {menu_router} m ON m.path = ml.router_path
      WHERE ml.menu_name = :menu
      ORDER BY p1 ASC, p2 ASC, p3 ASC, p4 ASC, p5 ASC, p6 ASC, p7 ASC, p8 ASC, p9 ASC";
    $result = db_query($sql, array(':menu' => $menu_name), array('fetch' => PDO::FETCH_ASSOC));
    $links = array();
    foreach ($result as $item) {
      $item['href'] = $item['link_path'];
      $links[] = $item;
    }
    $items = menu_tree_data($links);
    $node_links = array();
    menu_tree_collect_node_links($items, $node_links);
    $menu_admin = TRUE;
    menu_tree_check_access($items, $node_links);
    $menu_admin = FALSE;
    $trees[$menu_name] = $items;
  }
  return $trees[$menu_name];
}

function megamenus_get_menu_item($menu_name, $mlid) {
  $tree = megamenus_get_tree($menu_name);
  if (function_exists('i18n_menu_localize_tree')) {
    $tree = i18n_menu_localize_tree($tree);
  }
  $item = megamenus_find_menu_item($tree, $mlid);
  return $item;
}

function megamenus_find_menu_item($tree, $mlid) {
  foreach($tree as $item) {
    if ($item['link']['mlid'] == $mlid) {
      return $item;
    }
    else {
      $result = megamenus_find_menu_item($item['below'], $mlid);
      if ($result) {
        return $result;
      }
    }
  }
  return NULL;
}

function megamenus_load_block($block_key) {
  $blocks = megamenus_get_blocks_info();
  return isset($blocks[$block_key]) ? $blocks[$block_key] : NULL;
}

function megamenus_load_item($item, $level) {
  $li_config = new stdClass();
  $li_config->submenu = new stdClass();
  $li_config->submenu->rows_cols = array();
  $li_config->submenu->config = array(
    'width' => '',
  );
  $li_config->config = new stdClass();
  $li_config->config->hidesub = 1;
  $li_config->config->group = 0;
  $li_config->config->mlid = $item['link']['mlid'];
  $li_config->config->level = $level;
  $li_config->config->{'align-submenu'} = 0;  
  if (!empty($item['below'])) {
    $items = $item['below'];
    $temp = new stdClass();
    $temp->block = null;
    $temp->ul = array();
    $temp->type = 'ul';


    $temp->config = new stdClass();
    $temp->config->grid = '';
    $temp->config->level = $level;
    $temp->config->class = '';

    foreach ($items as $value) {
      if (!$value['link']['hidden']) {
        $temp->ul[] = megamenus_load_item($value, $level + 1);
      }
    }

    if($temp->block || !empty($temp->ul)) {
      $li_config->submenu = new stdClass();
      $li_config->submenu->rows_cols = array(0 => array(0 => new stdClass()));
      $li_config->submenu->config = array(
        'width' => '',
        'class' => '',
      );        
      $li_config->submenu->rows_cols[0][0] = $temp;
      $li_config->config = new stdClass();
      $li_config->config->group = 0;
      $li_config->config->hidesub = 0; 
      $li_config->config->mlid = $item['link']['mlid'];
      $li_config->config->level = $level;
      $li_config->config->{'align-submenu'} = 0;
    }
  }

  return $li_config;
}

function megamenus_get_struct_menu_from_menu_config($menu_config, $check_sub) {
  $my_menu_struct = array();
  foreach ($menu_config as $key_menu_config => $value_menu_config) {

    $my_menu_struct[$value_menu_config->config->mlid] = array();

    if (!empty($value_menu_config->submenu->rows_cols)) {
      foreach ($value_menu_config->submenu->rows_cols as $key => $value) {
        if (empty($value[0]->block)) {
          $my_menu_struct[$value_menu_config->config->mlid] = megamenus_get_struct_menu_from_menu_config($value[0]->ul, 0);
        }
      }
    }
  }

  return $my_menu_struct;
}

function megamenus_check_changed($mlid, $drupal_menu_item, $my_menu_item) {
  foreach ($my_menu_item as $key_my_menu_item => $value_my_menu_item) {
    
  }
}

function megamenus_block_content_exists($block_key, $section) {
  $block = megamenus_load_block($block_key);
  if($block) {
    $module = $block->module;
    $delta = $block->delta;
    $content = module_invoke($module, 'block_view', $delta);
    if(empty($content) && $section == 'backend') {
      return true;
    }
    elseif($content) {
      return true;
    }
  }
  return false;
}

function megamenus_sync_config($items, &$item_config, $_mlid, $section) {
  if(empty($item_config['rows_content'])) {
    $item_config['rows_content'] = array(0 => array(0 => array('col_content' => array(), 'col_config' => array())));
    foreach($items as $item) {
      $mlid = $item['link']['mlid'];
      if(!$item['link']['hidden']) {
        $item_config['rows_content'][0][0]['col_content'][] = array(
          'type' => 'menu_item',
          'mlid' => $mlid,
          'tb_item_config' => array(),
          'weight' => $item['link']['weight'],
        );
      }
    }
    if(empty($item_config['rows_content'][0][0]['col_content'])) {
      unset($item_config['rows_content'][0]);
    }
  }
  else {
    $hash = array();
    foreach($item_config['rows_content'] as $i => $row) {
      foreach($row as $j => $col) {
        foreach($col['col_content'] as $k => $tb_item) {
          if ($tb_item['type'] == 'menu_item') {
            $hash[$tb_item['mlid']] = array('row' => $i, 'col' => $j);
            $existed = false;
            foreach ($items as $item) {
              if (!$item['link']['hidden'] && $tb_item['mlid'] == $item['link']['mlid']) {
                $item_config['rows_content'][$i][$j]['col_content'][$k]['weight'] = $item['link']['weight'];
                $existed = true;
                break;
              }
            }
            if(!$existed) {
              unset($item_config['rows_content'][$i][$j]['col_content'][$k]);
              if(empty($item_config['rows_content'][$i][$j]['col_content'])) {
                unset($item_config['rows_content'][$i][$j]);
              }
              if(empty($item_config['rows_content'][$i])) {
                unset($item_config['rows_content'][$i]);
              }
            }
          }
          else {
            if(!megamenus_block_content_exists($tb_item['block_key'], $section)) {
              unset($item_config['rows_content'][$i][$j]['col_content'][$k]);
              if(empty($item_config['rows_content'][$i][$j]['col_content'])) {
                unset($item_config['rows_content'][$i][$j]);
              }
              if(empty($item_config['rows_content'][$i])) {
                unset($item_config['rows_content'][$i]);
              }
            }
          }
        }
      }
    }
    $row = -1; $col = -1;
    foreach($items as $item) {
      $mlid = $item['link']['mlid'];
      if(!$item['link']['hidden']) {
        if(isset($hash[$mlid])) {
          $row = $hash[$mlid]['row'];
          $col = $hash[$mlid]['col'];
          continue;
        }
        if($row > -1) {
          megamenus_insert_tb_item($item_config, $row, $col, $item);
        }
        else {
          $row = 0;
          $col = 0;
          while(isset($item_config['rows_content'][$row][$col]['col_content']) && $item_config['rows_content'][$row][$col]['col_content'][0]['type'] == 'block') {
            $row ++;
          }
          megamenus_insert_tb_item($item_config, $row, $col, $item);
          $item_config['rows_content'][$row][$col]['col_config'] = array();
        }
      }
    }
  }
}

function megamenus_insert_tb_item(&$item_config, $row, $col, $item) {
  $i = 0;
  $col_content = isset($item_config['rows_content'][$row][$col]['col_content']) ? $item_config['rows_content'][$row][$col]['col_content'] : array();
  while($i < count($col_content) && $col_content[$i]['weight'] < $item['link']['weight']) {
    $i ++;
  }
  for($j = count($col_content); $j > $i; $j --) {
    $item_config['rows_content'][$row][$col]['col_content'][$j] = $item_config['rows_content'][$row][$col]['col_content'][$j - 1];
  }
  $item_config['rows_content'][$row][$col]['col_content'][$i] = array(
    'mlid' => $item['link']['mlid'],
    'type' => 'menu_item',
    'weight' => $item['link']['weight'],
    'tb_item_config' => array(),
  );
}

function megamenus_array_to_object($array) {
  $obj = new stdClass;
  foreach($array as $k => $v) {
     if(is_array($v)) {
        $obj->{$k} = megamenus_array_to_object($v);
     } else {
        $obj->{$k} = $v;
     }
  }
  return $obj;
}

function megamenus_edit_item_config(&$item_config) {
  if(!isset($item_config['icon'])) {
    $item_config['icon'] = "";
  }
  if(!isset($item_config['class'])) {
    $item_config['class'] = "";
  }
  if(!isset($item_config['caption'])) {
    $item_config['caption'] = "";
  }
  if(!isset($item_config['align-submenu'])) {
    $item_config['align-submenu'] = "";
  }
  if(!isset($item_config['group'])) {
    $item_config['group'] = 0;
  }
  if(!isset($item_config['hidewcol'])) {
    $item_config['hidewcol'] = 0;
  }
  if(!isset($item_config['hidesub'])) {
    $item_config['hidesub'] = 0;
  }
}

function megamenus_edit_submenu_config(&$submenu_config) {
  if(!isset($submenu_config['width'])) {
    $submenu_config['width'] = "";
  }
  if(!isset($submenu_config['class'])) {
    $submenu_config['class'] = "";
  }
  if(!isset($submenu_config['group'])) {
    $submenu_config['group'] = "";
  }
}

function megamenus_edit_col_config(&$col_config) {
  if(!isset($col_config['width'])) {
    $col_config['width'] = 12;
  }
  if(!isset($col_config['class'])) {
    $col_config['class'] = "";
  }
  if(!isset($col_config['hidewcol'])) {
    $col_config['hidewcol'] = 0;
  }
  if(!isset($col_config['showblocktitle'])) {
    $col_config['showblocktitle'] = 0;
  }
}

function megamenus_edit_block_config(&$block_config) {
  if(!isset($block_config->animation)) {
    $block_config->animation = "none";
  }
  if(!isset($block_config->style)) {
    $block_config->style = "";
  }
  if(!isset($block_config->{'auto-arrow'})) {
    $block_config->{'auto-arrow'} = true;
  }
  if(!isset($block_config->duration)) {
    $block_config->duration = 400;
  }
  if(!isset($block_config->delay)) {
    $block_config->delay = 200;
  }
  if(!isset($block_config->{'always-show-submenu'})) {
    $block_config->{'always-show-submenu'} = 1;
  }
  if(!isset($block_config->{'off-canvas'})) {
    $block_config->{'off-canvas'} = 0;
  }

}

function megamenus_render_block($delta = 0) {
  global $tb_elements_counter;
  $tb_elements_counter = array('column' => 0);
  $block = array('content' => array(
    '#type' => 'markup',
    '#markup' => theme('megamenus', array('menu_name' => $delta, 'section' => 'backend',)),
  ));
  drupal_add_js('Drupal.MegaMenus = Drupal.MegaMenus || {};', array('type' => 'inline'));
  drupal_add_js('Drupal.MegaMenus.ElementsCounter = ' . json_encode($tb_elements_counter), array('type' => 'inline'));
  drupal_add_js('Drupal.MegaMenus.ElementsCounter = ' . json_encode($tb_elements_counter), array('type' => 'inline'));
  return $block;
}

function megamenus_animation_options($block_config) {
  $animations = array('none' => t('None'), 'fading' => t('Fading'), 'slide' => t('Slide'), 'zoom' => t('Zoom'), 'elastic' => t('Elastic'));
  $options = array();
  foreach($animations as $value => $title) {
    if($value == $block_config->animation) {
      $options[] = '<option value="' . $value . '" selected="selected">' . $title . '</option>';
    }
    else {
      $options[] = '<option value="' . $value . '">' . $title . '</option>';
    }
  }
  return implode("\n", $options);
}

function megamenus_direction_options($block_config) {
  $styles = array(
    'horizontal' => 'Horizontal',
    'vertical' => 'Vertical',
  );
  $options = array();
  foreach($styles as $value => $title) {
    if(isset($block_config->direction) && $value == $block_config->direction) {
      $options[] = '<option value="' . $value . '" selected="selected">' . $title . '</option>';
    }
    else {
      $options[] = '<option value="' . $value . '">' . $title . '</option>';
    }
  }
  return implode("\n", $options);

}

function megamenus_preset_options($block_config) {
  $styles = array(
    '' => 'None',
    'style_blue' => 'Blue',
    'style_classic' => 'Classic',
    'style_red' => 'Red',
    'theme_weebpal_education' => 'WeebPal Education',
    'theme_weebpal_corporate_site' => 'WeebPal Corporate Site',
    'theme_weebpal_hotel' => 'WeebPal Hotel',
    'theme_weebpal_ishopping' => 'WeebPal iShopping',
    'theme_weebpal_zircon' => 'WeebPal Zircon',    
    'theme_business' => 'Business',
    'theme_flatize' => 'Flatize',
    'theme_new_shopping' => 'New Shopping',
  );


  $link_preset_file = drupal_get_path('module', 'megamenus') . "/stylesheets/presets/presets.ini";

  if (file_exists($link_preset_file)) {
    # File exists
    $array_preset_file = explode("\n", file_get_contents($link_preset_file));

    $arr_preset_result = array();
    foreach ($array_preset_file as $key => $value) {
      if (!empty($value)) {
        $arr_value_preset = explode(':', $value);
        $arr_preset_result[$arr_value_preset[0]] = $arr_value_preset[1];
      }
    }
    
    $styles = array_merge($styles, $arr_preset_result);
  } 

  

  $options = array();
  foreach($styles as $value => $title) {
    if($value == $block_config->style) {
      $options[] = '<option value="' . $value . '" selected="selected">' . $title . '</option>';
    }
    else {
      $options[] = '<option value="' . $value . '">' . $title . '</option>';
    }
  }
  return implode("\n", $options);
}

function megamenus_find_hook_templates($cache, $path) {
  $templates = array();
  $regex = '/\.tpl\.php$/';

  // Because drupal_system_listing works the way it does, we check for real
  // templates separately from checking for patterns.
  $files = drupal_system_listing($regex, $path, 'name', 0);
  if(isset($files['megamenus-submenu--default.tpl'])) {
    unset($files['megamenus-submenu--default.tpl']);
  }
  foreach ($files as $template => $file) {
    // Chop off the remaining extensions if there are any. $template already
    // has the rightmost extension removed, but there might still be more,
    // such as with .tpl.php, which still has .tpl in $template at this point.
    if (($pos = strpos($template, '.')) !== FALSE) {
      $template = substr($template, 0, $pos);
    }
    // Transform - in filenames to _ to match function naming scheme
    // for the purposes of searching.
    $hook = strtr($template, '-', '_');
    if (isset($cache[$hook])) {
      $templates[$hook] = array(
        'template' => $template,
        'path' => dirname($file->uri),
        'includes' => isset($cache[$hook]['includes']) ? $cache[$hook]['includes'] : NULL,
      );
    }
    // Ensure that the pattern is maintained from base themes to its sub-themes.
    // Each sub-theme will have their templates scanned so the pattern must be
    // held for subsequent runs.
    if (isset($cache[$hook]['pattern'])) {
      $templates[$hook]['pattern'] = $cache[$hook]['pattern'];
    }
  }

  $patterns = array_keys($files);

  foreach ($cache as $hook => $info) {
    if (!empty($info['pattern'])) {
      // Transform _ in pattern to - to match file naming scheme
      // for the purposes of searching.
      $pattern = strtr($info['pattern'], '_', '-');

      $matches = preg_grep('/^'. $pattern .'/', $patterns);
      if ($matches) {
        foreach ($matches as $match) {
          $file = substr($match, 0, strpos($match, '.'));
          // Put the underscores back in for the hook name and register this pattern.
          $templates[strtr($file, '-', '_')] = array(
            'template' => $file,
            'path' => dirname($files[$match]->uri),
            'variables' => isset($info['variables']) ? $info['variables'] : NULL,
            'render element' => isset($info['render element']) ? $info['render element'] : NULL,
            'base hook' => $hook,
            'includes' => isset($info['includes']) ? $info['includes'] : NULL,
          );
        }
      }
    }
  }
  return $templates;
}

function megamenus_is_a_key($str) {
  $no_keys = array('(', ')', ';', '/', '#megamenus', '@');
  foreach($no_keys as $key) {
    if(strpos($str, $key) !== false) {
      return false;
    }
  }
  $yes_keys = array('.', '-', ',', '{');
  foreach($yes_keys as $key) {
    if(strpos($str, $key) !== false) {
      return true;
    }
  }
  return false;
}

function megamenus_replace_css() {
  $files = array(
    drupal_get_path('module', 'megamenus') . '/stylesheets/base.css',
    drupal_get_path('module', 'megamenus') . '/stylesheets/backend.css',
    drupal_get_path('module', 'megamenus') . '/stylesheets/megamenus.css',
    drupal_get_path('module', 'megamenus') . '/stylesheets/presets/black.css',
    drupal_get_path('module', 'megamenus') . '/stylesheets/presets/blue.css',
    drupal_get_path('module', 'megamenus') . '/stylesheets/presets/green.css',
    drupal_get_path('module', 'megamenus') . '/stylesheets/presets/orange.css',
  );
  foreach($files as $css_file) {
    $output = array();
    $f = fopen($css_file, "r");
    $lines = array();
    $ls = array();
    while($line = fgets($f)) {
      $lines[] = $line;
      $line = str_replace("\r\n", "", $line);
      if(megamenus_is_a_key($line)) {
        $line = trim($line);
        if(strpos($line, '.megamenus') !== 0) {
          $ls[] = '.megamenus ' . $line;
        }
        else {
          $ls[] = $line;
        }
      }
      else {
        $ls[] = $line;
      }
    }
    file_put_contents($css_file, implode("\r\n", $ls));
    fclose($f);
  }
}

function megamenus_get_counter($key) {
  $value = &drupal_static($key, 0);
  $value ++;
  global $tb_elements_counter;
  if(!$tb_elements_counter) {
    $tb_elements_counter = array();
  }
  $tb_elements_counter[$key] = $value;
  return "megamenus-$key-$value";
}

function megamenus_check_exits_tb_mega_menu ($menu_name) {
  if (db_table_exists('tb_megamenus')) {
    $tb_mega_menu = db_select('tb_megamenus', 't')
                  ->fields('t')
                  ->condition('menu_name', $menu_name,'=')
                  ->execute()
                  ->fetchAssoc();
    if (empty($tb_mega_menu['menu_name'])) {
      return 0;
    }
    return 1;
  }

  return 0;
}

function megamenus_check_imported ($menu_name) {
  $tb_mega_menu = db_select('tb_megamenus', 't')
                  ->fields('t')
                  ->condition('menu_name', $menu_name,'=')
                  ->execute()
                  ->fetchAssoc();

  if (empty($tb_mega_menu['block_config']['import_from_tb_menu'])) {
    return 1;
  }

  return 0;
}

function megamenus_is_different($items_m, $items_d) {
  $i = 0;
  foreach($items_d as $i2) {
    if(!$i2['link']['hidden']) {
      
      $i1 = isset($items_m[$i]) ? $items_m[$i] : NULL;
      $i ++;
      if(isset($i1->config->mlid) && $i1->config->mlid != $i2['link']['mlid']) {
        return true;
      }
    }
  }

  if($i < count($items_m)) {
    return true;
  }
  
  $i = 0;
  foreach($items_d as $i2) {
    if(!$i2['link']['hidden']) {
      $i1 = isset($items_m[$i]) ? $items_m[$i] : NULL;
      $i ++;
      if(megamenus_is_different_submenus($i1, $i2, TRUE)) {
        return true;
      }
    }
  }

  return false;
}

function megamenus_get_subitems($megamenu_item) {
  $items = array();
  if(isset($megamenu_item->submenu->rows_cols)) {
    $rows = $megamenu_item->submenu->rows_cols;
    foreach($rows as $row) {
      foreach($row as $col) {
        if($col->type === 'ul') {
          foreach($col->ul as $item) {
            $items[] = $item;
          }
        }
      }
    }
  }
  return $items;
}

function megamenus_is_different_submenus($item_m, $item_d, $recursive = TRUE) {
  $items_m = megamenus_get_subitems($item_m);
  $items_d = $item_d['below'];
  $i = 0;
  foreach($items_d as $i2) {
    if(!$i2['link']['hidden']) {
      $i1 = isset($items_m[$i]) ? $items_m[$i] : NULL;
      $i ++;
      if(!$i1 || $i1->config->mlid != $i2['link']['mlid']) {
        return true;
      }
    }
  }
  if($i < count($items_m)) {
    return true;
  }
  if($recursive) {
    $i = 0;
    foreach($items_d as $i2) {
      if(!$i2['link']['hidden']) {
        $i1 = isset($items_m[$i]) ? $items_m[$i] : NULL;
        $i ++;
        if(megamenus_is_different_submenus($i1, $i2)) {
          return true;
        }
      }
    }
  }
  return false;
}

function megamenus_reset_from_drupalmenu($megamenu, $items) {
  if(megamenus_is_different($megamenu, $items)) {
    $menu = array();
    foreach ($items as $key => $value) {
      if (!$value['link']['hidden']) {
        $menu[] = megamenus_load_item($value, 1);
      }
    }
  }
  return $menu;
}

function megamenus_merge_with_drupalmenu($megamenu, $items) {
  if(megamenus_is_different($megamenu, $items)) {
    $new_megamenu = array();
    foreach ($items as $key => $item) {
      if (!$item['link']['hidden']) {
        $menu_item = megamenus_find_submenu($item['link']['mlid'], $megamenu);
        if(megamenus_is_different_submenus($menu_item, $item, TRUE)) {
          $new_megamenu[] = megamenus_load_item($item, 1);
        }
        else {
          $new_megamenu[] = $menu_item;
        }
      }
    }
    return $new_megamenu;
  }
  return $megamenu;
}

function megamenus_find_submenu($mlid, $megamenu) {
  foreach($megamenu as $item) {
    if($item->config->mlid == $mlid) {
      return $item;
    }
    if(isset($item->submenu->rows_cols)) {
      $rows = $item->submenu->rows_cols;
      foreach($rows as $row) {
        foreach($row as $col) {
          if($col->type == 'ul') {
            $result = megamenus_find_submenu($mlid, $col->ul);
            if($result) {
              return $result;
            }
          }
        }
      }
    }
  }
  return NULL;
}

<?php

include_once drupal_get_path('module', 'megamenus') . '/megamenus.inc';

function megamenus_admin_settings($form) {
  $menus = megamenus_get_megamenus();
  foreach ($menus as $menu) {
    $form[$menu->menu_name]['#megamenus'] = $menu;
    $form[$menu->menu_name]['menu_name'] = array('#markup' => $menu->menu_name);
    $form[$menu->menu_name]['title'] = array('#markup' => $menu->title);
    $form[$menu->menu_name]['config_megamenu'] = array('#type' => 'link', '#title' => t('Config'), '#href' => "admin/structure/megamenus/$menu->menu_name");
    $form[$menu->menu_name]['config_links'] = array('#type' => 'link', '#title' => t('Edit links'), '#href' => "admin/structure/menu/manage/$menu->menu_name");

    # If exits tb_menu & and have never import
    if (megamenus_check_exits_tb_mega_menu($menu->menu_name) &&
        megamenus_check_imported($menu->menu_name)) {
      $form[$menu->menu_name]['import_tb_mega_menu'] = array('#type' => 'link', '#title' => t('Import From TB Mega Menu'), '#href' => "admin/structure/megamenus/$menu->menu_name/convert");
    }
  }
  return $form;
}

function theme_megamenus_admin_settings($variables) {
  $form = $variables['form'];
  $rows = array();
  foreach (element_children($form) as $key) {
    if (isset($form[$key]['#megamenus'])) {
      $menu = &$form[$key];
      $menu['enabled']['#parents'] = array($key, 'enabled');
      $menu['enabled']['#name'] = $key . "[enabled]";
      $row = array();
      $row[] = drupal_render($menu['menu_name']);
      $row[] = drupal_render($menu['title']);
      $row[] = drupal_render($menu['config_megamenu']);
      $row[] = drupal_render($menu['config_links']);
      $row[] = drupal_render($menu['import_tb_mega_menu']);
      $rows[] = $row;
    }
  }
  $header = array(t('Menu Name'), t('Menu Title'));
  $header[] = array('data' => t('Operations'), 'colspan' => 3);
  return theme('table', array('header' => $header, 'rows' => $rows, 'empty' => t('No MegaMenu block available. <a href="@link">Add MegaMenu Block</a>.', array('@link' => url('admin/config/user-interface/megamenus/add'))), 'attributes' => array('id' => 'megamenus'))) . drupal_render_children($form);
}

function megamenus_configure_form($form, &$form_state, $menu_names = array()) {
  megamenus_add_css(drupal_get_path('module', 'megamenus') . '/stylesheets/base.css');
  megamenus_add_css(drupal_get_path('module', 'megamenus') . '/stylesheets/backend.css');  
  drupal_add_js(array('ajax_link' => (variable_get('clean_url', 0) ? '' : '?q=')), 'setting');  
  
  drupal_add_js(drupal_get_path('module', 'megamenus') . '/js/megamenus.js');
  if(!empty($menu_names)) {
    $form['megamenus'] = array(
      '#markup' => theme('megamenus_backend', array(
        'menu_name' => $menu_names[0],
      )),
    );
  }
  return $form;
}

function megamenus_convert_from_tb_menu_confirm($form, &$form_state, $menu_names = array()) {
  $menu_name = $menu_names[0];

  if (user_access('administer megamenus')) {
    $form['menu_name'] = array(
      '#type'  => 'hidden',
      '#value' => $menu_name,
    );

    $output = confirm_form($form,
      t('Are you sure you want to convert from Mega TB Menu %title?', array('%title' => $menu_name)),
      isset($_GET['destination']) ? $_GET['destination'] : 'admin/structure/megamenus');
  }
  return $output;
}

function megamenus_convert_from_tb_menu_confirm_submit($form, &$form_state) {
  megamenus_convert_from_tb_menu($form_state['input']['menu_name']);
  $form_state['redirect'] = 'admin/structure/megamenus';
  drupal_set_message('Import Success !', $type = 'status');
}

function megamenus_convert_from_tb_menu($menu_name) {
  $tb_mega_menu = db_select('tb_megamenus', 't')
    ->fields('t')
    ->condition('menu_name', $menu_name,'=')
    ->execute()
    ->fetchAssoc();
  $tb_mega_menu_config = $tb_mega_menu['menu_config'];
  $tb_mega_block_config = $tb_mega_menu['block_config'];
  $tb_mega_menu_config = json_decode($tb_mega_menu_config, true);
  $tb_mega_block_config = json_decode($tb_mega_block_config, true);
  //file_put_contents("public://tb_megamenus.txt", print_r($tb_mega_menu_config, true));

  $menu_struct = menu_tree_all_data($menu_name);
  $new_menu_config = array();
  $new_block_config = array();

  foreach ($menu_struct as $key => $value) {
    $new_menu_config[] = megamenus_convert_menu_config($value, 1, $tb_mega_menu_config);
  }

  $new_block_config = megamenus_convert_block_config($tb_mega_block_config);

  $new_menu_config = json_encode($new_menu_config);
  $new_block_config = json_encode($new_block_config);

  $megamenus = db_select('megamenus', 't')
    ->fields('t')
    ->condition('menu_name', $menu_name,'=')
    ->execute()
    ->fetchAssoc();
  
  if (empty($megamenus['menu_name'])) {
    $nid = db_insert('megamenus')
      ->fields(array(
        'menu_name' => $menu_name,
        'block_config' => $new_block_config,
        'menu_config' => $new_menu_config,
      ))
      ->execute();
  } else {
    $num_updated = db_update('megamenus')
      ->fields(array(
        'block_config' => $new_block_config,
        'menu_config' => $new_menu_config,
      ))
      ->condition('menu_name', $menu_name, '=')
      ->execute();
  }
}

function megamenus_convert_block_config($tb_mega_block_config) {
  $result = new stdClass();
  $result->direction = 'horizontal';
  $result->style = $tb_mega_block_config['style'];
  $result->animation = $tb_mega_block_config['animation'];
  $result->{'auto-arrow'} = $tb_mega_block_config['auto-arrow'];
  $result->{'always-show-submenu'} = $tb_mega_block_config['always-show-submenu'];
  $result->duration = $tb_mega_block_config['duration'];
  $result->delay = $tb_mega_block_config['delay'];
  $result->import_from_tb_menu = '1';
  return $result;
}

function megamenus_check_menu_break ($value_rows) {
  $result = new stdClass();
  $result->break = 0;
  $result->cols = array();

  $arr_cols_break = array();
  foreach ($value_rows as $key_rows_break => $value_rows_break) {
    if ($value_rows_break['col_content'][0]['type'] == 'menu_item') {
      $arr_cols_break[$key_rows_break] = $value_rows_break['col_content'][0]['mlid'];
    }
  }

  $count = 0;
  foreach ($arr_cols_break as $key => $value) {
    if ($value != 0) {
      $count++;
    }
  }

  $result->break = $count > 1 ? 1 : 0;
  $result->cols = $arr_cols_break;

  return $result;
}

function megamenu_get_next_mlid($list_mlids) {
  static $start_position = 0;
  $next_mlid = NULL;
  for($i = $start_position; $i < count($list_mlids); $i ++) {
    $start_position ++;
    $mlid = $list_mlids[$i];
    if($mlid) {
      return $mlid;
    }
  }
  return $next_mlid;
}

function megamenu_get_list_first_mlids($rows) {
  $list_first_mlids = array();
    
  foreach ($rows as $key_row => $row) {
    foreach ($row as $key_col => $col) {
      if ($col['col_content'][0]['type'] == 'menu_item') {
        $list_first_mlids[] = $col['col_content'][0]['mlid'];
      }
      else {
        $list_first_mlids[] = NULL;
      }
    }
  }
  return $list_first_mlids;
}

function megamenus_convert_menu_config($item, $level, $tb_mega_menu_config) {
  $arr_past = array();
  $cur_menu_id = $item['link']['mlid'];

  $cur_item_config = array();
  $cur_submenu_config = array();
  $cur_rows_content = array();

  if (!empty($tb_mega_menu_config[$cur_menu_id])) {
    $cur_item_config = $tb_mega_menu_config[$cur_menu_id]['item_config'];
    $cur_submenu_config = $tb_mega_menu_config[$cur_menu_id]['submenu_config'];
    $cur_rows_content = $tb_mega_menu_config[$cur_menu_id]['rows_content'];
  }

  $li_config = new stdClass();
  if (!empty($item['below']) || !empty($cur_rows_content)) {
    $sub_items = $item['below'];
    $arr_rows_cols = array();
    $list_first_mlids = megamenu_get_list_first_mlids($cur_rows_content);
    $next_mlid = megamenu_get_next_mlid($list_first_mlids);

    foreach ($cur_rows_content as $row) {
      $arr_cols = array();
      
      foreach ($row as $col) {
        $cur_rows_cols = new stdClass();
        
        if ($col['col_content'][0]['type'] === 'menu_item') {
          $first_mlid = $next_mlid;
          $next_mlid = megamenu_get_next_mlid($list_first_mlids);
          $cur_rows_cols->block = null;
          $cur_rows_cols->ul = array();
          $cur_rows_cols->type = 'ul';

          $in_list = false;
          foreach($sub_items as $sub_item) {
            if(!$first_mlid) {
              $first_mlid = $sub_item['link']['mlid'];
            }
            if($sub_item['link']['mlid'] == $first_mlid) {
              $in_list = true;
            }
            if($sub_item['link']['mlid'] == $next_mlid) {
              break;
            }
            if($in_list) {
              $cur_rows_cols->ul[] = megamenus_convert_menu_config($sub_item, $level + 1, $tb_mega_menu_config);
            }
          }
        } else {
          $cur_rows_cols->block = new stdClass();
          $cur_rows_cols->block->block = $col['col_content'][0]['block_key'];
          $cur_rows_cols->ul = null;
          $cur_rows_cols->type = 'block';
        }
        
        $cur_rows_cols->config = new stdClass();
        $cur_rows_cols->config->hidewcol = $cur_item_config['hidewcol'];
        $cur_rows_cols->config->grid = $col['col_config']['width'];
        $cur_rows_cols->config->level = $level;
        $cur_rows_cols->config->class = $col['col_config']['class'];
        $cur_rows_cols->config->{'show-block-title'} = $col['col_config']['showblocktitle'];
        
        $arr_cols[] = $cur_rows_cols;
      }
      $arr_rows_cols[] = $arr_cols;
    }
    $li_config->config = new stdClass();
    $li_config->config->{'align-submenu'} = !empty($cur_item_config['alignsub']) ? $cur_item_config['alignsub'] : 'left';
    $li_config->config->hidewsub = isset($cur_item_config['hidesub']) ? $cur_item_config['hidesub'] : '';
    $li_config->config->group = isset($cur_item_config['group']) ? $cur_item_config['group'] : '';
    $li_config->config->{'a-class'} = '';
    $li_config->config->mlid = $item['link']['mlid'];
    $li_config->config->level = $level;
    $li_config->submenu = new stdClass();
    if(!empty($arr_rows_cols)) {
      $li_config->submenu->config = array(
        'width' => $cur_submenu_config['width'],
        'class' => $cur_submenu_config['class'],
      );
      $li_config->submenu->rows_cols = $arr_rows_cols;
    }
    else {
      $li_config->config->hidewsub = 1;
    }

  } else {
    if (!empty($cur_submenu_config['width']) || !empty($cur_submenu_config['class'])) {
      $li_config->submenu = new stdClass();
      $li_config->submenu->rows_cols = array();
      $li_config->submenu->config = array(
        'width' => $cur_submenu_config['width'],
        'class' => $cur_submenu_config['class'],
      );
    } else {
      $li_config->submenu = null;
    }
    
    $li_config->config = new stdClass();
    $li_config->config->{'align-submenu'} = !empty($cur_item_config['alignsub']) ? $cur_item_config['alignsub'] : 0;
    $li_config->config->hidewsub = isset($cur_item_config['hidesub']) ? $cur_item_config['hidesub'] : '' ;
    $li_config->config->group = isset($cur_item_config['group']) ? $cur_item_config['group'] : '' ;
    $li_config->config->{'a-class'} = '';
    $li_config->config->mlid = $item['link']['mlid'];
    $li_config->config->level = $level;
  }
  
  return $li_config;
}
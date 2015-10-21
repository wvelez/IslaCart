<?php
include_once(drupal_get_path('theme', 'marketplace') . '/common.inc');

function marketplace_theme() {
  $items = array();
  $items['render_panel'] = array(
    "variables" => array(
      'page' => array(),
      'panels_list' => array(),
      'panel_regions_width' => array(),
    ),
    'preprocess functions' => array(
      'marketplace_preprocess_render_panel'
    ),
    'template' => 'panel',
    'path' => drupal_get_path('theme', 'marketplace') . '/tpl',
  );

  $items['user_login_block'] = array(
    'template' => 'user-login-block',
    'render element' => 'form',
    'path' => drupal_get_path('theme', 'marketplace') . '/tpl',
    'preprocess functions' => array(
      'marketplace_preprocess_user_login_block'
    ),
  );

  return $items;
}

function marketplace_preprocess_user_login_block(&$vars) {
  $vars['form']['links']['#markup'] = '<div class="links">Not a member? <a href="'. base_path() . 'user/register">Sign up now</a></div>';
  $vars['name'] = render($vars['form']['name']);
  $vars['pass'] = render($vars['form']['pass']);
  $vars['submit'] = render($vars['form']['actions']['submit']);
  $vars['rendered'] = drupal_render_children($vars['form']);
}

function marketplace_process_html(&$vars) {
  $current_skin = theme_get_setting('skin');
  if (isset($_COOKIE['weebpal_skin'])) {
    $current_skin = $_COOKIE['weebpal_skin'];
  }
  if (!empty($current_skin) && $current_skin != 'default') {
    $vars['classes'] .= " skin-$current_skin";
  }

  $current_background = theme_get_setting('background');
  if (isset($_COOKIE['weebpal_background'])) {
    $current_background = $_COOKIE['weebpal_background'];
  }
  if (!empty($current_background)) {
    $vars['classes'] .= ' ' . $current_background;
  }

}

function marketplace_preprocess_page(&$vars) {
  global $theme_key;

  $vars['page_css'] = '';

  $vars['regions_width'] = marketplace_regions_width($vars['page']);
  $panel_regions = marketplace_panel_regions();
  if (count($panel_regions)) {
    foreach ($panel_regions as $panel_name => $panels_list) {
      $panel_markup = theme("render_panel", array(
        'page' => $vars['page'],
        'panels_list' => $panels_list,
        'regions_width' => $vars['regions_width'],
      ));
      $panel_markup = trim($panel_markup);
      $vars['page'][$panel_name] = empty($panel_markup) ? FALSE : array('content' => array('#markup' => $panel_markup));
    }
  }

  $current_skin = theme_get_setting('skin');
  if (isset($_COOKIE['weebpal_skin'])) {
    $current_skin = $_COOKIE['weebpal_skin'];
  }

  $layout_width = (theme_get_setting('layout_width') == '')
                  ? theme_get_setting('layout_width_default')
                  : theme_get_setting('layout_width');
  $vars['page']['show_skins_menu'] = $show_skins_menu = theme_get_setting('show_skins_menu');

  if($show_skins_menu) {
    $current_layout = theme_get_setting('layout');
    if (isset($_COOKIE['weebpal_layout'])) {
      $current_layout = $_COOKIE['weebpal_layout'];
    }

    if ($current_layout == 'layout-boxed') {
      $vars['page_css'] = 'style="max-width:' . $layout_width . 'px;margin: 0 auto;" class="boxed"';
    }
    $data = array(
      'layout_width' => $layout_width,
      'current_layout' => $current_layout
    );
    $skins_menu = theme_render_template(drupal_get_path('theme', 'marketplace') . '/tpl/skins-menu.tpl.php', $data);
    $vars['page']['show_skins_menu'] = $skins_menu;
  }

  $vars['page']['weebpal_skin_classes'] = !empty($current_skin) ? ($current_skin . "-skin") : "";
  if (!empty($current_skin) && $current_skin != 'default' && theme_get_setting("default_logo") && theme_get_setting("toggle_logo")) {
    $vars['logo'] = file_create_url(drupal_get_path('theme', $theme_key)) . "/css/colors/" . $current_skin . "/images/logo.png";
  }

  //////////////////////////////////////

  $skin = theme_get_setting('skin');
  if (isset($_COOKIE['weebpal_skin'])) {
    $skin = $_COOKIE['weebpal_skin'] == 'default' ? '' : $_COOKIE['weebpal_skin'];
  }
  if (!empty($skin)) {
    $pathfile = drupal_get_path('theme', $theme_key) . "/css/colors/" . $skin . "/style.css";
    if (file_exists($pathfile)) {
      drupal_add_css($pathfile, array(
        'group' => CSS_THEME,
      ));
    }
  }

  drupal_add_css('http://fonts.googleapis.com/css?family=Open+Sans:400,300,600,700,800', array(
    'group' => CSS_SYSTEM,
    'weight' => -10,
    'type' => 'external'
  ));
}

function marketplace_preprocess_node(&$vars) {
  if ($vars['type'] == 'blog') {
    $vars['title_link'] = FALSE;
    if (in_array('node-teaser', $vars['classes_array']) || in_array('node-preview', $vars['classes_array'])) {
      $vars['title_link'] = TRUE;
      if (isset($vars['content']['field_blog_type']['#object']->field_blog_type['und'][0]['taxonomy_term']->name)) {
        $vars['blog_type'] = $vars['content']['field_blog_type']['#object']->field_blog_type['und'][0]['taxonomy_term']->name;
        unset($vars['content']['field_blog_type']);
      }
    }

    $vars['marketplace_media_field'] = false;
    foreach($vars['content'] as $key => $field) {
      if (isset($field['#field_type']) && isset($field['#weight'])) {
        if ($field['#field_type'] == 'image' || $field['#field_type'] == 'video_embed_field' || $field['#field_type'] == 'youtube') {
          $vars['marketplace_media_field'] = drupal_render($field);
          $vars['classes_array'][] = 'marketplace-media-first';
          unset($vars['content'][$key]);
          break;
        }
      }
    }

    $vars['page'] = ($vars['type'] == 'page') ? TRUE : FALSE;
    $vars['created_date'] = date('M d, Y', $vars['created']);

    if(isset($vars['content']['links']['comment'])) {
      $vars['comment_links'] = $vars['content']['links']['comment'];
      unset($vars['content']['links']['comment']);
    }
  }
}

function marketplace_preprocess_render_panel(&$variables) {
    $page = $variables['page'];
    $panels_list = $variables['panels_list'];
    $regions_width = $variables['regions_width'];
    $variables = array();
    $variables['page'] = array();
    $variables['panel_width'] = $regions_width;
    $variables['panel_classes'] = array();
    $variables['panels_list'] = $panels_list;
    $is_empty = TRUE;
    $panel_keys = array_keys($panels_list);

    foreach ($panels_list as $panel) {
        $variables['page'][$panel] = $page[$panel];
        $panel_width = $regions_width[$panel];
        if (render($page[$panel])) {
            $is_empty = FALSE;
        }
        $classes = array("panel-column");
        $classes[] = "col-sm-$panel_width";
        $classes[] = str_replace("_", "-", $panel);
        $variables['panel_classes'][$panel] = implode(" ", $classes);
    }
    $variables['empty_panel'] = $is_empty;
}

function marketplace_preprocess_views_view_fields(&$vars) {
  $view = $vars['view'];
  foreach ($vars['fields'] as $id => $field) {
    if(isset($field->handler->field_info) && $field->handler->field_info['type'] === 'image') {
      $prefix = $field->wrapper_prefix;
      if(strpos($prefix, "views-field ") !== false) {
        $parts = explode("views-field ", $prefix);
        $type = str_replace("_", "-", $field->handler->field_info['type']);
        $prefix = implode("views-field views-field-type-" . $type . " ", $parts);
      }
      $vars['fields'][$id]->wrapper_prefix = $prefix;
    }
  }
}

function marketplace_breadcrumb($variables) {
  $alias = drupal_get_path_alias();
  $breadcrumb = $variables['breadcrumb'];

  if (!empty($breadcrumb)) {
    // Provide a navigational heading to give context for breadcrumb links to
    // screen-reader users. Make the heading invisible with .element-invisible.
    if (strpos($alias, 'comment/reply') !== false) {
      unset($breadcrumb[1]);
    }
    $output = '<h2 class="element-invisible">' . t('You are here') . '</h2>';

    $output .= '<div class="breadcrumb">' . implode('<span></span>', $breadcrumb) . '</div>';
    return $output;
  }
}

/**
 * Implements hook_element_info_alter
 */

function marketplace_element_info_alter(&$info) {
  $info['select']['#pre_render'][] = 'marketplace_render_select';
}

function marketplace_render_select($element) {
  global $language;
  if($language->direction == 1 && in_array('chosen', $element['#attached']['library'][0])) {
    $element['#attributes']['class'][] = 'chosen-select';
    $element['#attributes']['class'][] = 'chosen-rtl';
  }
  return $element;
}
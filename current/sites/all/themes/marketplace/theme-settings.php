<?php
/**
 * @file
 * Theme setting callbacks for the marketplace theme.
 */
include_once(drupal_get_path('theme', 'marketplace') . '/common.inc');

function marketplace_reset_settings() {
  global $theme_key;
  variable_del('theme_' . $theme_key . '_settings');
  variable_del('theme_settings');
  $cache = &drupal_static('theme_get_setting', array());
  $cache[$theme_key] = NULL;
}

function marketplace_form_system_theme_settings_alter(&$form, $form_state) {
  if (theme_get_setting('marketplace_use_default_settings')) {
    marketplace_reset_settings();
  }
  $form['#attached']['js'][] = array(
    'data' => drupal_get_path('theme', 'marketplace') . '/js/weebpal.js',
    'type' => 'file',
  );
  $form['marketplace']['marketplace_version'] = array(
    '#type' => 'hidden',
    '#default' => '1.0',
  );
  marketplace_settings_layout_tab($form);
  marketplace_feedback_form($form);
  $form['#submit'][] = 'marketplace_form_system_theme_settings_submit';
}

function marketplace_settings_layout_tab(&$form) {
  global $theme_key;
  $skins = marketplace_get_predefined_param('skins', array('' => t("Default skin")));
  $backgrounds = marketplace_get_predefined_param('backgrounds', array('bg-default' => t("Default")));
  $layout = marketplace_get_predefined_param('layout', array('layout-default' => t("Default Layout")));

  $form['marketplace']['settings'] = array(
    '#type' => 'fieldset',
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#title' => t('Settings'),
    '#weight' => 0,
  );

  if (count($skins) > 1) {
    $form['marketplace']['settings']['configs'] = array(
      '#type' => 'fieldset',
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#title' => t('Configs'),
      '#weight' => 0,
    );
    $form['marketplace']['settings']['configs']['skin'] = array(
      '#type' => 'select',
      '#title' => t('Skin'),
      '#default_value' => theme_get_setting('skin'),
      '#options' => $skins,
    );
  }

  $form['marketplace']['settings']['configs']['background'] = array(
    '#type' => 'select',
    '#title' => t('Background'),
    '#default_value' => theme_get_setting('background'),
    '#options' => $backgrounds,
    '#weight' => 1,
  );

  $form['marketplace']['settings']['configs']['layout'] = array(
    '#type' => 'select',
    '#title' => t('Layout'),
    '#default_value' => theme_get_setting('layout'),
    '#options' => $layout,
    '#weight' => -2,
  );
  $default_layout_width = (theme_get_setting('layout_width') == '') ? '1400' : theme_get_setting('layout_width');
  $form['marketplace']['settings']['configs']['layout_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Layout Width(px)'),
    '#default_value' => $default_layout_width,
    '#size' => 15,
    '#require' => TRUE,
    '#weight' => -1,
    '#states' => array(
      'visible' => array(
        'select[name="layout"]' => array(
          'value' => 'layout-boxed',
        ),
      ),
    ),
  );

  $form['theme_settings']['toggle_logo']['#default_value'] = theme_get_setting('toggle_logo');
  $form['theme_settings']['toggle_name']['#default_value'] = theme_get_setting('toggle_name');
  $form['theme_settings']['toggle_slogan']['#default_value'] = theme_get_setting('toggle_slogan');
  $form['theme_settings']['toggle_node_user_picture']['#default_value'] = theme_get_setting('toggle_node_user_picture');
  $form['theme_settings']['toggle_comment_user_picture']['#default_value'] = theme_get_setting('toggle_comment_user_picture');
  $form['theme_settings']['toggle_comment_user_verification']['#default_value'] = theme_get_setting('toggle_comment_user_verification');
  $form['theme_settings']['toggle_favicon']['#default_value'] = theme_get_setting('toggle_favicon');
  $form['theme_settings']['toggle_secondary_menu']['#default_value'] = theme_get_setting('toggle_secondary_menu');
  $form['theme_settings']['show_skins_menu'] = array(
    '#type' => 'checkbox',
    '#title' => t('Show Skins Menu'),
    '#default_value' => theme_get_setting('show_skins_menu'),
  );
  $form['theme_settings']['loading_page'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use loading'),
    '#default_value' => theme_get_setting('loading_page'),
  );

  $form['logo']['default_logo']['#default_value'] = theme_get_setting('default_logo');
  $form['logo']['settings']['logo_path']['#default_value'] = theme_get_setting('logo_path');
  $form['favicon']['default_favicon']['#default_value'] = theme_get_setting('default_favicon');
  $form['favicon']['settings']['favicon_path']['#default_value'] = theme_get_setting('favicon_path');
  $form['theme_settings']['#collapsible'] = TRUE;
  $form['theme_settings']['#collapsed'] = FALSE;
  $form['logo']['#collapsible'] = TRUE;
  $form['logo']['#collapsed'] = FALSE;
  $form['favicon']['#collapsible'] = TRUE;
  $form['favicon']['#collapsed'] = FALSE;
  $form['marketplace']['settings']['theme_settings'] = $form['theme_settings'];
  $form['marketplace']['settings']['logo'] = $form['logo'];
  $form['marketplace']['settings']['favicon'] = $form['favicon'];

  unset($form['theme_settings']);
  unset($form['logo']);
  unset($form['favicon']);

  $form['marketplace']['marketplace_use_default_settings'] = array(
    '#type' => 'hidden',
    '#default_value' => 0,
  );
  $form['actions']['marketplace_use_default_settings_wrapper'] = array(
    '#markup' => '<input type="submit" value="' . t('Reset theme settings') . '" class="form-submit form-reset" onclick="return Drupal.Light.onClickResetDefaultSettings();" style="float: right;">',
  );
}

function marketplace_feedback_form(&$form) {
  $form['marketplace']['about_marketplace'] = array(
    '#type' => 'fieldset',
    '#title' => t('Feedback Form'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#weight' => 40,
  );

  $form['marketplace']['about_marketplace']['about_marketplace_wrapper'] = array(
    '#type' => 'container',
    '#attributes' => array('class' => array('about-marketplace-wrapper')),
  );

  $form['marketplace']['about_marketplace']['about_marketplace_wrapper']['about_marketplace_content'] = array(
    '#markup' => '<iframe width="100%" height="650" scrolling="no" class="nucleus_frame" frameborder="0" src="http://www.weebpal.com/static/feedback/"></iframe>',
  );
}

function marketplace_form_system_theme_settings_submit($form, &$form_state) {
  if(isset($form_state['input']['skin']) && $form_state['input']['skin'] != $form_state['complete form']['marketplace']['settings']['configs']['skin']['#default_value']) {
    setcookie('weebpal_skin', $form_state['input']['skin'], time() + 100000, base_path());
  }
  if (isset($form_state['input']['background']) && $form_state['input']['background'] !== $form_state['complete form']['marketplace']['settings']['configs']['background']['#default_value']) {
    setcookie('weebpal_background', $form_state['input']['background'], time() + 100000, base_path());
  }
  if (isset($form_state['input']['layout']) && $form_state['input']['layout'] !== $form_state['complete form']['marketplace']['settings']['configs']['layout']['#default_value']) {
    setcookie('weebpal_layout', $form_state['input']['layout'], time() + 100000, base_path());
  }
}

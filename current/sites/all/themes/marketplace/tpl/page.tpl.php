<?php
/**
 * @file
 * Hermio's theme implementation to display a single Drupal page.
 *
 * The doctype, html, head and body tags are not in this template. Instead they
 * can be found in the html.tpl.php template in this directory.
 *
 * Available variables:
 *
 * General utility variables:
 * - $base_path: The base URL path of the Drupal installation. At the very
 *   least, this will always default to /.
 * - $directory: The directory the template is located in, e.g. modules/system
 *   or themes/bartik.
 * - $is_front: TRUE if the current page is the front page.
 * - $logged_in: TRUE if the user is registered and signed in.
 * - $is_admin: TRUE if the user has permission to access administration pages.
 *
 * Site identity:
 * - $front_page: The URL of the front page. Use this instead of $base_path,
 *   when linking to the front page. This includes the language domain or
 *   prefix.
 * - $logo: The path to the logo image, as defined in theme configuration.
 * - $site_name: The name of the site, empty when display has been disabled
 *   in theme settings.
 * - $site_slogan: The slogan of the site, empty when display has been disabled
 *   in theme settings.
 *
 * Navigation:
 * - $main_menu (array): An array containing the Main menu links for the
 *   site, if they have been configured.
 * - $secondary_menu (array): An array containing the Secondary menu links for
 *   the site, if they have been configured.
 * - $breadcrumb: The breadcrumb trail for the current page.
 *
 * Page content (in order of occurrence in the default page.tpl.php):
 * - $title_prefix (array): An array containing additional output populated by
 *   modules, intended to be displayed in front of the main title tag that
 *   appears in the template.
 * - $title: The page title, for use in the actual HTML content.
 * - $title_suffix (array): An array containing additional output populated by
 *   modules, intended to be displayed after the main title tag that appears in
 *   the template.
 * - $messages: HTML for status and error messages. Should be displayed
 *   prominently.
 * - $tabs (array): Tabs linking to any sub-pages beneath the current page
 *   (e.g., the view and edit tabs when displaying a node).
 * - $action_links (array): Actions local to the page, such as 'Add menu' on the
 *   menu administration interface.
 * - $feed_icons: A string of all feed icons for the current page.
 * - $node: The node object, if there is an automatically-loaded node
 *   associated with the page, and the node ID is the second argument
 *   in the page's path (e.g. node/12345 and node/12345/revisions, but not
 *   comment/reply/12345).
 *
 * Regions:
 * - $page['help']: Dynamic help text, mostly for admin pages.
 * - $page['highlighted']: Items for the highlighted content region.
 * - $page['content']: The main content of the current page.
 * - $page['sidebar_first']: Items for the first sidebar.
 * - $page['sidebar_second']: Items for the second sidebar.
 * - $page['header']: Items for the header region.
 * - $page['footer']: Items for the footer region.
 *
 * @see template_preprocess()
 * @see template_preprocess_page()
 * @see template_process()
 * @see html.tpl.php
 *
 * @ingroup themeable
 */
?>
  <?php if($panel_mobile_menu = render($page['panel_mobile_menu'])): ?>
    <div id="mobile-menu" class="uk-offcanvas">
      <div class="uk-offcanvas-bar uk-offcanvas-bar-flip">
        <?php print $panel_mobile_menu; ?>
      </div>
    </div>
  <?php endif; ?>

  <div id="page" <?php print $page_css ?>>
    <?php if(isset($page['show_skins_menu']) && $page['show_skins_menu']):?>
      <?php print $page['show_skins_menu'];?>
    <?php endif;?>

    <?php if($headline = render($page['headline'])): ?>
      <section id="headline" class="headline">
        <div class="container">
          <?php print $headline; ?>
        </div>
      </section>
    <?php endif;?>

    <header id="header" class="header section">
      <div class="container">

        <?php if ($logo): ?>
          <a href="<?php print $front_page; ?>" title="<?php print t('Back to Home'); ?>" rel="home" id="logo">
            <img src="<?php print $logo; ?>" alt="<?php print t('Back to Home'); ?>" />
          </a>
        <?php endif; ?>

        <?php if ($site_name || $site_slogan): ?>
          <div id="name-and-slogan">
            <?php if ($site_name): ?>
              <?php if ($title): ?>
                <div id="site-name"><strong>
                  <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
                </strong></div>
              <?php else: /* Use h1 when the content title is empty */ ?>
                <h1 id="site-name">
                  <a href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>" rel="home"><span><?php print $site_name; ?></span></a>
                </h1>
              <?php endif; ?>
            <?php endif; ?>

            <?php if ($site_slogan): ?>
              <div id="site-slogan"><?php print $site_slogan; ?></div>
            <?php endif; ?>
          </div>
        <?php endif; ?>

        <?php print render($page['header']); ?>

        <?php if($main_menu = render($page['main_menu'])): ?>
          <nav class="collapse navbar-collapse width" id="main-menu-inner">
            <?php print $main_menu; ?>
          </nav>
        <?php endif;?>

        <a href="javascript:;" class="off-canvas-toggle icon-toggle" data-uk-offcanvas="{target:'#mobile-menu'}">
          <i class="fa fa-navicon"></i>
        </a>

      </div>
    </header>

    <?php if (($menu_top = render($page['panel_menu_top'])) || $breadcrumb): ?>
      <section id="section-top" class="section">
          <div class="container">
            <?php print $menu_top; ?>
            <?php print $breadcrumb; ?>
          </div>
      </section>
    <?php endif; ?>


    <?php if ($messages): ?>
      <section id="messages" class="message section">
        <div class="container">
          <?php print $messages; ?>
        </div>
      </section>
    <?php endif;?>

    <section id="main" class="main section">
      <div class="container">
        <div class="row">
          <?php if ($regions_width['sidebar_first']): ?>
            <aside id="sidebar-first" class="sidebar col-lg-<?php print $regions_width['sidebar_first']?> col-md-<?php print $regions_width['sidebar_first']?> col-sm-12 col-xs-12">
              <div class="section">
                <?php print render($page['sidebar_first']); ?>
              </div>
            </aside>
          <?php endif; ?>
          <div class="col-md-<?php print $regions_width['content']?> col-sm-12 col-xs-12">
            <?php if ($panel_highlighted = render($page['panel_highlighted'])): ?>
              <div id="highlighted" class="section">
                <?php print $panel_highlighted; ?>
              </div>
            <?php endif; ?>
            <?php if($panel_first = render($page['panel_first'])): ?>
              <div id="panel-first" class="section">
                  <?php print $panel_first;?>
              </div>
            <?php endif; ?>
            <div id="content" class="column">
      				<?php if ($title): ?>
      				  <?php print render($title_prefix); ?>
      				  <?php if ($title): ?><h1 class="title" id="page-title"><?php print $title; ?></h1><?php endif; ?>
      				  <?php print render($title_suffix); ?>
      				<?php endif; ?>
              <a id="main-content"></a>
              <?php if ($tabs): ?><div class="tabs"><?php print render($tabs); ?></div><?php endif; ?>
              <?php print render($page['help']); ?>
              <?php if ($action_links): ?><ul class="action-links"><?php print render($action_links); ?></ul><?php endif; ?>
              <?php print render($page['content']); ?>
            </div>
          </div>
          <?php if ($page['sidebar_second']): ?>
            <aside id="sidebar-second" class="sidebar col-lg-<?php print $regions_width['sidebar_second']?> col-md-<?php print $regions_width['sidebar_second']?> col-sm-12 col-xs-12">

                <?php print render($page['sidebar_second']); ?>

            </aside>
          <?php endif; ?>
        </div>
      </div>
    </section>

    <?php if($panel_second = render($page['panel_second'])): ?>
      <section id="panel-second" class="section">
        <div class="container">
          <?php print $panel_second;?>
        </div>
      </section>
    <?php endif; ?>

    <?php if($panel_third = render($page['panel_third'])): ?>
      <section id="panel-third" class="section">
        <div class="container">
          <?php print $panel_third;?>
        </div>
      </section>
    <?php endif; ?>

    <?php if($panel_footer = render($page['panel_footer'])): ?>
      <section id="panel-footer" class="section">
        <div class="container">
          <?php print $panel_footer;?>
        </div>
      </section>
    <?php endif; ?>

    <?php if($footer = render($page['footer'])): ?>
      <footer id="footer" class="section">
        <div class="container">
          <?php print $footer; ?>
        </div>
      </footer>
    <?php endif;?>
    <a title="<?php print t('Back to Top')?>" class="btn-btt" href="#Top"></a>
  </div>


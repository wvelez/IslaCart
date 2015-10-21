<div <?php print $attributes;?> class="navbar navbar-megamenus <?php print $classes;?>">
  <div class="container-fluid">
    <div class="navbar-header">
      <button type="button" class="navbar-toggle" data-toggle="collapse" data-target="#<?php print $menu_name;?>">
        <span class="sr-only">Toggle navigation</span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
        <span class="icon-bar"></span>
      </button>
    </div>

    <div class="collapse navbar-collapse" id="<?php print $menu_name;?>">
      <?php print $content;?>
    </div>
  </div>
</div>

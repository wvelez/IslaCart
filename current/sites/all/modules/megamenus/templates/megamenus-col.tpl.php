<div <?php print $attributes;?> class="<?php print $classes;?>">
  <div class="megamenus-col-inner">
    <?php if(isset($close_button)): ?>
      <?php print $close_button;?>
    <?php endif;?>
    <?php print $tb_items;?>
  </div>
</div>

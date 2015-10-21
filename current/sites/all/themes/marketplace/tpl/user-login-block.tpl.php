<div id="user-login-block-container">
  <div id="user-login-block-form-fields">
    <?php print $name; ?>
    <?php print $pass; ?>
    <div class="newpass"><a href="<?php echo base_path() . 'user/password' ?>">Forgot password?</a></div>
    <?php print $submit; ?>
    <?php print $rendered; ?>
  </div>
</div>
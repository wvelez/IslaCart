(function ($) {

Drupal.Marketplace = Drupal.Marketplace || {};

Drupal.behaviors.actionMarketplace = {
  attach: function (context) {
    $('.btn-btt').smoothScroll({speed: 600});
    $(".scroll-to").smoothScroll({speed: 600});
    Drupal.Marketplace.initProductQuantity();
    Drupal.Marketplace.productDetail();
    Drupal.Marketplace.searchBlock();
    Drupal.Marketplace.shoppingCart();
    Drupal.Marketplace.hidePopup();
    Drupal.Marketplace.showPopup();
    $(".field-brand .field-item").once('load').prepend("<span>by </span>");
    Drupal.Marketplace.productViewType();
    Drupal.Marketplace.productImageActive();
    Drupal.Marketplace.fixCarousel();

    //Placeholder
    Drupal.Marketplace.setInputPlaceHolder('mail', 'Your email', '.simplenews-subscribe .form-item-mail');
    Drupal.Marketplace.setInputPlaceHolder('search_block_form', 'Search...', '#block-search-form');
    $(window).resize(function() {
      var menumobile_show = $('a.off-canvas-toggle.icon-toggle').css('display');
      if (menumobile_show == 'block') {
        $("#mobile-menu").trigger("click");
      }
    });
  }
};

Drupal.Marketplace.shoppingCart = function () {
  var cart_block = $("#block-commerce-cart-cart");
  if (cart_block.find(".cart-empty-block").length == 0) {
    cart_block.find(".line-item-summary .line-item-quantity-label").text("Cart");
    var cart_popup = cart_block.find(".view");
    if(cart_popup.find('.cart-content').length == 0)
    {
        cart_popup.append('<div class="cart-content"></div>')
    }
    if(cart_popup.find('.cart-content .cart-title').length == 0){
        cart_popup.find('.cart-content').append('<h5 class="cart-title">Your cart</h5>');
    }

    cart_popup.find('.view-content').appendTo(cart_popup.find('.cart-content'));
    cart_popup.find('.view-footer').appendTo(cart_popup.find('.cart-content'));
  }
  else {
    cart_block.find(".cart-empty-block").html('<span class="line-item-quantity-raw">0</span><span class="line-item-quantity-label load-processed">Cart</span>');
    cart_block.find(".cart-empty-block").append('<div class="cart-content">Your cart is empty</div>');
  }
};


Drupal.Marketplace.initProductQuantity = function () {
  var stock = $(".product-stock div.field-item");
  stock.hide();
  var instock = stock.text();
  instock = parseInt(instock);
  var quantity = $(".commerce-add-to-cart .form-item-quantity");
  if(quantity.children('.commerce-add-to-cart .increase').length == 0) {
    quantity.append($('<span class="btn increase" id="quantity-increase"></span>'));
  }
  if(quantity.children('.commerce-add-to-cart .decrease').length == 0) {
    quantity.prepend($('<span class="btn decrease" id="quantity-decrease"></span>'));
  }
  var node_product_price = $("#main .node .field-name-field-product .form-item-quantity");
  if(node_product_price.find('.increase').length == 0){
    node_product_price.append('<span class="btn increase" id="quantity-increase"></span>');
  }
  if(node_product_price.find('.decrease').length == 0){
    node_product_price.prepend('<span class="btn decrease" id="quantity-decrease"></span>');
  }
  $('#quantity-increase').once('load').click(function(event){
    var value = parseInt($(this).parent().children('input#edit-quantity').val());
    value = value + 1;
    if (value <= instock) {
      $(this).parent().children('input#edit-quantity').val(value);
      $(this).parent().children('.commerce-add-to-cart .decrease').removeClass("disabled");
    }
    event.preventDefault();
    event.stopPropagation();
  });

  node_product_price.find('.decrease').once('load').click(function(event) {
    var value = parseInt($(this).parent().find('.form-text').val()) - 1;
    if (value >= 1) {
      $(this).parent().find('.form-text').val(value);
      $(this).parent().children('.commerce-add-to-cart .increase').removeClass("disabled");
      if (value == 1) {
        $(this).parent().children('.commerce-add-to-cart .decrease').addClass("disabled");
      }
    }
    event.preventDefault();
    event.stopPropagation();
  });

    var outStock = $('.out-of-stock');
    outStock.find('.form-item-quantity .form-text').prop('disabled', true);

    /* shopping cart detail */
    var cart_quantity = $('#views-form-commerce-cart-form-default .views-field-edit-quantity .form-item');

    $('#views-form-commerce-cart-form-default tbody tr').each(function(){
        var stock = $(this).find('td.views-field-edit-quantity span').hide().text();
        if($(this).find('.increase').length == 0){
            cart_quantity.append('<a href="javascript:void(0)" class="btn increase"></a>');
        }
        if($(this).find('.decrease').length == 0){
            cart_quantity.prepend('<a href="javascript:void(0)" class="btn decrease"></a>');
        }
        $(this).find('.increase').once('load').click(function () {
          var value = parseInt($(this).parent().find('input[type=text]').val()) + 1;
          if (value <= stock) {
            $(this).parent().find('input[type=text]').val(value);
          }
        });
        $(this).find('.decrease').once('load').click(function(){
            var value = parseInt($(this).parent().find('input[type=text]').val());
            if(value > 1){
                value--;
                $(this).parent().find('input[type=text]').val(value);
            }
        });
    });

};

Drupal.Marketplace.productDetail = function () {
  if ($("#product-more-info").length != 0) {
    if($("#product-info-tabs").length == 0) {
      var tabs = $('<div class="product-info-tabs" id="product-info-tabs"></div>');
      $("#product-more-info").before(tabs);
      tabs = $("#product-info-tabs");
      if ($("#product-detail").length != 0) {
        tabs.append('<a id="hover-product-detail" href="#product-detail">Product Details</a>');
        $('#hover-product-detail').smoothScroll({speed: 600});
      }

      if ($("#production-specifications").length != 0) {
        tabs.append('<a id="hover-production-specifications" href="#production-specifications">Production Specifications</a>');
        $('#hover-production-specifications').smoothScroll({speed: 600});
      }

      if ($("#comments").length != 0) {
        tabs.append('<a id="hover-comments" href="#comments-region">Reviews</a>');
        $('#hover-comments').smoothScroll({speed: 600});
      }
      else if ($(".links").length != 0) {
        tabs.append('<a id="hover-comments" href="#comments-region">Reviews</a>');
        $('#hover-comments').smoothScroll({speed: 600});
      }
    }
  }
};

Drupal.Marketplace.searchBlock = function(){
    var selected = $('.search-block .views-widget-filter-tid');
    if(selected.find('.views-widget .selected-value').length == 0){
      selected.find('.views-widget').prepend('<div class="selected-value">' +
        '<span class="search-label"></span><i class="fa fa-caret-down"></i></div>');
    }


    var text = $('.search-block .form-select option:selected').text();
    selected.find('.search-label').text(text);

    $('.search-block .form-select').once('load').change(function(){
        var text = $('.search-block .form-select option:selected').text();
        selected.find('.search-label').text(text);
    });

    $('.search-block .views-submit-button .form-submit').val('Go');
}

Drupal.Marketplace.hidePopup = function () {
  $('#page').once('load').click(function () {
    $('.user-login-popup-block, .block-menu-descriptions').removeClass('show');
    $('#block-commerce-cart-cart').removeClass('show');
  });
};

Drupal.Marketplace.showPopup = function(){
  $("#block-user-login .content").click(function (event) {
    event.stopPropagation();
  });
  $(".block-view-orders .content").click(function (event) {
    event.stopPropagation();
  });
  $('#block-commerce-cart-cart .cart-content').click(function(e){
      e.stopPropagation();
  });
  $('.block-menu-descriptions, #block-user-login').once('load').click(function (event) {
    $('.block.show').removeClass('show');
    $(this).toggleClass('show');
    event.stopPropagation();
  });
  $('#block-commerce-cart-cart').once('load').click(function (event) {
    $('.block.show').removeClass('show');
    $(this).toggleClass('show');
    event.stopPropagation();
  });
  $(".block-view-orders").once("load").click(function (event) {
    console.log("OK");
    $('.block.show').removeClass('show');
    $(this).toggleClass('show');
    event.stopPropagation();
  })
};

Drupal.Marketplace.productViewType = function () {
  $(".product-display-type").once('load').click(function (event) {
    $(".product-display-type").removeClass("active");
    $(this).addClass("active");
    var display_type = $(this).data("type");
    $(".views-product").removeClass("products-grid").removeClass("products-list");
    $(".views-product").addClass(display_type);
    event.preventDefault();
  });
};

Drupal.Marketplace.productImageActive = function () {
  $(".cloud-zoom-gallery-thumbs a:first-child").addClass("active");
  $(".cloud-zoom-gallery-thumbs a").once('load').click(function (event) {
    $(".cloud-zoom-gallery-thumbs a").removeClass("active");
    $(this).addClass('active');
  });
}

Drupal.Marketplace.setInputPlaceHolder = function (name, text, selector) {
  selector = selector == undefined ? '' : selector + ' ';

  if ($.support.placeholder) {
    $(selector + 'input[name="' + name + '"]').attr('placeholder', Drupal.t(text));
  }
  else {
    $(selector + 'input[name="' + name + '"]').val(Drupal.t(text));
    $(selector + 'input[name="' + name + '"]').focus(function(){
      if(this.value == Drupal.t(text)) {
        this.value='';
      }
    }).blur(function(){
      if(this.value == '') {
        this.value=Drupal.t(text);
      }
    });
  }
}

Drupal.Marketplace.fixCarousel = function () {
  $(".jcarousel-prev").removeAttr("disabled").removeClass("jcarousel-prev-disabled");
  $(".jcarousel-next").removeAttr("disabled").removeClass("jcarousel-next-disabled");
}

})(jQuery);
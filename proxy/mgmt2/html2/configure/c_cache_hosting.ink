<!-------------------------------------------------------------------------
  
  ------------------------------------------------------------------------->

<@include /include/header.ink>
<@include /configure/c_header.ink>

<form method=POST action="/submit_update.cgi?<@link_query>">
<input type=hidden name=record_version value=<@record_version>>
<input type=hidden name=submit_from_page value=<@link_file>>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr class="tertiaryColor"> 
    <td class="greyLinks"> 
      <p>&nbsp;&nbsp;Cache Hosting Configuration</p>
    </td>
  </tr>
</table>

<@include /configure/c_buttons_hide.ink>

<@submit_error_msg>

<table width="100%" border="0" cellspacing="0" cellpadding="10"> 
  <tr>
    <td width="100%" nowrap class="configureLabel" valign="top">
      <@submit_error_flg proxy.config.cache.hosting_filename>Cache Hosting
    </td>
  </tr>
  <tr>
    <td width="100%" class="configureHelp" valign="top" align="left"> 
      The "<@record proxy.config.cache.hosting_filename>" file lets
      you assign cache partitions to specific origin servers and/or
      domains so that you can manage your cache space more
      efficiently, and restrict disk usage.  Use in conjunction with
      "<@record proxy.config.cache.partition_filename>".
    </td>
  </tr>
  <tr>
   <td>
    <@config_table_object /configure/f_hosting_config.ink>
   </td>
  </tr>
  <tr>
    <td colspan="2" align="right">
     <input class="configureButton" type=button name="refresh" value="Refresh" onclick="window.location='/configure/c_cache_hosting.ink?<@link_query>'">
     <input class="configureButton" type=button name="editFile" value="Edit File" target="displayWin" onclick="window.open('/configure/submit_config_display.cgi?filename=/configure/f_hosting_config.ink&fname=<@record proxy.config.cache.hosting_filename>&frecord=proxy.config.cache.hosting_filename', 'displayWin');">
    </td>
  </tr>
</table>

<@include /configure/c_buttons_hide.ink>
<@include /configure/c_footer.ink>

</form>

<@include /include/footer.ink>

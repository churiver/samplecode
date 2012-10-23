package com.oauthapp.model;

import java.util.EnumMap;

import org.scribe.builder.ServiceBuilder;
import org.scribe.model.Token;
import org.scribe.model.Verifier;
import org.scribe.oauth.OAuthService;

import com.oauthapp.util.Constant;
import com.oauthapp.util.Constant.DirectoryProvider;
import com.oauthapp.util.Constant.ConfigType;

/**
 * @author yul
 * 
 */
public class OauthManager
{
    private EnumMap<DirectoryProvider, OAuthService> services = null;
    private OAuthService service = null;
    private Token requestToken = null;
    private String sid = null;


    public OauthManager(String sessionID)
    {
        services = new EnumMap<DirectoryProvider, OAuthService>(DirectoryProvider.class);
        sid = sessionID;

        // Using the Scribe library to begin the chain of Oauth2 calls.
        for (DirectoryProvider dirProvider : DirectoryProvider.values())
        {
            // The callback param is for user to jump back to oauthapp
            // from Linkedin grant permission page
            OAuthService service = new ServiceBuilder()
                    .provider(Constant.API_CLASSES.get(dirProvider))
                    .apiKey(Constant.API_KEYS.get(dirProvider))
                    .apiSecret(Constant.API_SECRETS.get(dirProvider))
                    .callback(
                            Constant.URL_WEBDIR + "/config/user?" + Constant.PARAM_CONFIG_GRANT + "="
                                    + dirProvider.toString() + "&sid=" + sid).build();
            services.put(dirProvider, service);
        }
    }

    /*******************************************************
     * Get authorization URL
     * 
     * @param dirProvider
     * @return
     */
    public String getUrl(DirectoryProvider dirProvider, ConfigType urlType)
    {
        String url = null;

        switch (urlType)
        {
            case GRANT:
                service = services.get(dirProvider);
                requestToken = service.getRequestToken();
                url = service.getAuthorizationUrl(requestToken);
                break;
            case REVOKE:
                url = Constant.URL_OAUTHAPP + "/config/user?" + Constant.PARAM_CONFIG_REVOKE + "="
                        + dirProvider.toString();
                break;
            default:
                url = "Not-Set-Yet";
        }
        return url;

    }

    /******************************************************
     * Get accessToken from Oauth API
     * 
     * @param vcode
     * @return
     */
    public Token getAccessToken(String vcode) throws Exception
    {
        
        Verifier verifier = new Verifier(vcode);
        return service.getAccessToken(requestToken, verifier);
    }

}

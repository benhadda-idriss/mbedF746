#include <mbed.h>
#include <threadLvgl.h>
#include <widgets/lv_bar.h>
#include "demos/lv_demos.h"
#include <cstdio>
#include <stdio.h>
#include "DFRobot_BloodOxygen_S.h"

ThreadLvgl threadLvgl(30);
// I2C i2c(D14, D15);
char response[1];
char command[1] = {0xff}; // Demande pour la température

DFRobot_BloodOxygen_S_I2C capteur;

lv_obj_t *scrDonnee;
lv_obj_t *scrMesure;
lv_obj_t *barreSPO2;
lv_obj_t *labelBarreSPO2;
lv_obj_t *barreHeartBeat;
lv_obj_t *labelBarreHeartBeat;
lv_obj_t *labelButtonMesure;
lv_obj_t *buttonMesure;
lv_obj_t *labelButtonDonnee;
lv_obj_t *buttonDonnee;
lv_obj_t *buttonPrincipal;
lv_obj_t *labelButtonPrincipal;
lv_obj_t *chart;
lv_chart_series_t *ser1;
lv_chart_series_t *ser2;
lv_obj_t *labelResultSpo2;
lv_obj_t *labelResultHeart;
lv_obj_t *barreMesure;
lv_style_t style_green;
lv_style_t style_white;

int spo2 = 0;
int heartbeat = 0;
float temperature;
char labelTextSpo2[32];
char labelTextHeartbeat[32];
int tabSpo2[30];
int tabHeartbeat[30];
bool startMeasure = false;
bool afficheDonnee = false;
int jourSPO2[10];
int jourheartbeat[10];
int moyenneHeartbeat = 0;
int moyenneSpo2 = 0;

int i = 0;
int j = 0;

void create_scrDonnee(void);
void create_scrMesure(void);
static void event_handler_buttonMesure(lv_event_t *e);
static void event_handler_buttonDonnee(lv_event_t *e);
static void event_handler_buttonPrincipal(lv_event_t *e);

int main()
{
    for (i = 0; i < 31; i++)
    {
        jourSPO2[i] =0;
        jourheartbeat[i] = 0;
    }

    threadLvgl.lock();
    create_scrDonnee();
    create_scrMesure();

    lv_scr_load(scrMesure);
    threadLvgl.unlock();

    while (1)
    {
        if (startMeasure) // Vérifiez si la mesure doit commencer
        {
            moyenneHeartbeat=0;
            moyenneSpo2=0;
            capteur.sensorStartCollect();
            for (i = 0; i < 30; i++)
            {
                capteur.getHeartbeatSPO2();
                spo2 = capteur._sHeartbeatSPO2.SPO2;
                heartbeat = capteur._sHeartbeatSPO2.Heartbeat;
                temperature = capteur.getTemperature_C();
                if (spo2 != -1)
                {
                    tabSpo2[i] = spo2;
                }
                else
                {
                    tabSpo2[i] = tabSpo2[i - 1];
                }
                if (spo2 != -1)
                {
                    tabHeartbeat[i] = heartbeat;
                }
                else
                {
                    tabHeartbeat[i] = tabHeartbeat[i - 1];
                }

                sprintf(labelTextSpo2, "SPO2: %d%%", spo2);
                sprintf(labelTextHeartbeat, "Heartbeat: %d times/min", heartbeat);

                threadLvgl.lock();
                lv_bar_set_value(barreSPO2, spo2, LV_ANIM_ON);
                lv_bar_set_value(barreMesure, i, LV_ANIM_ON);
                lv_bar_set_value(barreHeartBeat, heartbeat, LV_ANIM_ON);
                lv_label_set_text(labelBarreSPO2, labelTextSpo2);
                lv_label_set_text(labelBarreHeartBeat, labelTextHeartbeat);
                lv_label_set_text(labelResultHeart, labelTextHeartbeat);
                lv_label_set_text(labelResultSpo2, labelTextSpo2);
                threadLvgl.unlock();

                printf("%d\n", i);

                moyenneHeartbeat = (tabHeartbeat[i] / 30) + moyenneHeartbeat;
                moyenneSpo2 = (tabSpo2[i] / 30) + moyenneSpo2;
                printf("mH= %d, mS= %d\n", moyenneHeartbeat, moyenneSpo2);
                ThisThread::sleep_for(2s);
            }
            capteur.sensorEndCollect();
            startMeasure = false; // Réinitialiser la variable après la mesure
            spo2 = 0;
            heartbeat = 0;
            sprintf(labelTextSpo2, "SPO2: %d%%", spo2);
            sprintf(labelTextHeartbeat, "Heartbeat: %d times/min", heartbeat);

            threadLvgl.lock();
            lv_bar_set_value(barreSPO2, spo2, LV_ANIM_ON);
            lv_bar_set_value(barreHeartBeat, heartbeat, LV_ANIM_ON);
            lv_label_set_text(labelBarreSPO2, labelTextSpo2);
            lv_label_set_text(labelBarreHeartBeat, labelTextHeartbeat);

            threadLvgl.unlock();
        }

        if (afficheDonnee)
        {
            jourheartbeat[j] = moyenneHeartbeat;
            jourSPO2[j] = moyenneSpo2;
            printf("jH= %d, jS= %d i=%d\n", jourheartbeat[j], jourSPO2[j], j);
            j++;
            threadLvgl.lock();
            for (i = 0; i < 10; i++)
            {
                /*Set the next points on 'ser1' from tab1*/
                lv_chart_set_next_value(chart, ser1, jourheartbeat[i]);

                lv_chart_set_next_value(chart, ser2, jourSPO2[i]);
                printf("jH = %d js= %d i=%d\n", jourheartbeat[i], jourSPO2[i], i);
            }
            threadLvgl.unlock();
            afficheDonnee = false;
        }

        ThisThread::sleep_for(100ms); // Donner du temps à l'interface utilisateur pour se mettre à jour
    }

    return 0;
}

void create_scrDonnee(void)
{
    threadLvgl.lock();
    scrDonnee = lv_obj_create(NULL);
    chart = lv_chart_create(scrDonnee);
    lv_obj_set_size(chart, 400, 100);
    lv_obj_set_pos(chart, 40, 0);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE); /*Show lines and points too*/

    /*Add two data series*/
    ser1 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    ser2 = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_SECONDARY_Y);
    lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 200);
    lv_chart_set_range(chart, LV_CHART_AXIS_SECONDARY_Y, 0, 120);

    lv_chart_refresh(chart); /*Required after direct set*/

    labelResultHeart = lv_label_create(scrDonnee);
    labelResultSpo2 = lv_label_create(scrDonnee);
    lv_obj_align_to(labelResultSpo2, chart, LV_ALIGN_OUT_BOTTOM_MID, -100, 100);
    lv_obj_align_to(labelResultHeart, chart, LV_ALIGN_OUT_BOTTOM_MID, 50, 100);

    // bouton pour commencer a mesurer
    buttonPrincipal = lv_btn_create(scrDonnee);
    lv_obj_add_event_cb(buttonPrincipal, event_handler_buttonPrincipal, LV_EVENT_ALL, NULL);
    lv_obj_align_to(buttonPrincipal, chart, LV_ALIGN_OUT_BOTTOM_MID, -80, 20);
    labelButtonPrincipal = lv_label_create(buttonPrincipal);
    lv_label_set_text(labelButtonPrincipal, "Retour menu mesure");
    lv_obj_center(labelButtonPrincipal);

    threadLvgl.unlock();
}

void create_scrMesure(void)
{
    threadLvgl.lock();

    scrMesure = lv_obj_create(NULL);
    barreSPO2 = lv_bar_create(scrMesure);
    labelBarreSPO2 = lv_label_create(scrMesure);
    barreHeartBeat = lv_bar_create(scrMesure);
    labelBarreHeartBeat = lv_label_create(scrMesure);
    barreMesure= lv_bar_create(scrMesure);
    

    lv_style_init(&style_green);
    lv_style_set_bg_color(&style_green, lv_color_make(0,255,0));
    lv_style_set_bg_opa(&style_green, LV_OPA_COVER);

lv_style_init(&style_white);
    lv_style_set_bg_color(&style_white, lv_color_make(255,255,255));
    lv_style_set_bg_opa(&style_white, LV_OPA_COVER);
    

    // barre et label du taux d'oxygene
    lv_label_set_text(labelBarreSPO2, "SPO2: 0%");
    lv_obj_set_size(barreSPO2, 200, 10);
    lv_obj_set_pos(barreSPO2, lv_obj_get_width(scrMesure) / 2 - 100, 50);
    lv_obj_align_to(labelBarreSPO2, barreSPO2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_bar_set_range(barreSPO2, 90, 100);
    lv_bar_set_value(barreSPO2, 0, LV_ANIM_ON);

    // barre et label de la frequence cardiaque
    lv_label_set_text(labelBarreHeartBeat, "Heartbeat: 0 times/min");
    lv_obj_set_size(barreHeartBeat, 200, 10);
    lv_obj_set_pos(barreHeartBeat, lv_obj_get_width(scrMesure) / 2 - 100, 150);
    lv_obj_align_to(labelBarreHeartBeat, barreHeartBeat, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    lv_bar_set_range(barreHeartBeat, 50, 200);
    lv_bar_set_value(barreHeartBeat, 0, LV_ANIM_ON);

    // bouton pour commencer a mesurer
    buttonMesure = lv_btn_create(scrMesure);
    lv_obj_add_event_cb(buttonMesure, event_handler_buttonMesure, LV_EVENT_ALL, NULL);
    lv_obj_align_to(buttonMesure, labelBarreSPO2, LV_ALIGN_OUT_BOTTOM_MID, -40, 20);

    labelButtonMesure = lv_label_create(buttonMesure);
    lv_label_set_text(labelButtonMesure, "Start Mesure");
    lv_obj_center(labelButtonMesure);

    // bouton pour commencer afficher les données
    buttonDonnee = lv_btn_create(scrMesure);
    lv_obj_add_event_cb(buttonDonnee, event_handler_buttonDonnee, LV_EVENT_ALL, NULL);
    lv_obj_align_to(buttonDonnee, labelBarreHeartBeat, LV_ALIGN_OUT_BOTTOM_MID, -30, 20);

    labelButtonDonnee = lv_label_create(buttonDonnee);
    lv_label_set_text(labelButtonDonnee, "résultat");
    lv_obj_center(labelButtonDonnee);

    lv_obj_set_size(barreMesure, 300, 10);
    lv_obj_set_pos(barreMesure, lv_obj_get_width(scrMesure) / 2 -200, 10);
    lv_bar_set_range(barreMesure, 0, 29);
    lv_obj_add_style(barreMesure, &style_green, LV_PART_INDICATOR);
    lv_obj_add_style(barreMesure, &style_white, LV_PART_MAIN);
    threadLvgl.unlock();
}

static void event_handler_buttonMesure(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        startMeasure = true; // Démarrer la mesure lorsque le bouton est cliqué
    }
}

static void event_handler_buttonDonnee(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {
        afficheDonnee = true;
        threadLvgl.lock();

        lv_scr_load(scrDonnee);

        threadLvgl.unlock();
    }
}

static void event_handler_buttonPrincipal(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED)
    {

        threadLvgl.lock();

        lv_scr_load(scrMesure);

        threadLvgl.unlock();
    }
}
